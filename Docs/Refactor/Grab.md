# 抓取系统重构方案

## 一、现有问题

### 1.1 职责混淆
当前设计中，`GrabComponent` 同时包含了：
- 属于**抓取者（Grabber）**的逻辑与属性
- 属于**被抓取物（Grabbee）**的逻辑与属性

### 1.2 平台耦合
`BP_MyMotionController` 继承自 `MotionControllerComponent`，其提供的 VR 手柄位置追踪功能在 PC 模式下不需要，导致 PC 模式难以复用现有逻辑。

---

## 二、重构目标

1. **分离 Grabber 与 Grabbee**：将抓取者的逻辑/属性与被抓取物的逻辑/属性彻底分离
2. **平台解耦**：支持 PC 和 VR 两种模式共用核心逻辑，各自实现差异部分
3. **C++ 化**：将基础逻辑从蓝图迁移到 C++

---

## 三、新类设计

### 3.1 Grabber 侧

| 类名 | 类型 | 说明 |
|------|------|------|
| `UPlayerGrabHand` | SceneComponent | 手部组件基类，取代 `BP_MyMotionController`，包含抓取逻辑 |
| `UPCGrabHand` | 继承 PlayerGrabHand | PC 模式专用，使用射线检测 FindTarget，包含 PC 端 Gravity Gloves 逻辑 |
| `UVRGrabHand` | 继承 PlayerGrabHand | VR 模式专用，使用球形检测 FindTarget，包含 VR 端 Gravity Gloves 逻辑 |

### 3.2 Grabbee 侧

| 类名 | 类型 | 说明 |
|------|------|------|
| `AGrabbeeObject` | Actor | 所有可抓取物体的基类 |
| `AGrabbeeWeapon` | 继承 GrabbeeObject | 武器基类，包含 WeaponType，用于 WeaponSnap 抓取 |

```
AGrabbeeObject          (Free, Snap, HumanBody, Custom)
    └── AGrabbeeWeapon  (WeaponSnap)
            ├── ABow
            └── AArrow
```

### 3.3 Player 侧

```
ABasePlayer
├── ABasePCPlayer    （PC 模式基类）
└── ABaseVRPlayer    （VR 模式基类）
```

---

## 四、GrabType 抓取类型

### 4.1 枚举定义

```cpp
UENUM(BlueprintType)
enum class EGrabType : uint8
{
    Free,        // 自由抓取，物理附加到手上
    Snap,        // 吸附抓取，对齐到固定位置/旋转
    WeaponSnap,  // 武器吸附，使用 GrabPoint 定位
    HumanBody,   // 人体拖拽，使用 PhysicsControl
    Custom       // 完全自定义，由子类重写 OnGrab/OnRelease
};
```

### 4.2 抓取实现方案

| GrabType | 实现方式 | 说明 |
|----------|----------|------|
| `Free` | PhysicsControl | 物体保持物理模拟，通过物理控制跟随手部，释放时自然保留物理速度（投掷手感好） |
| `Snap` | PhysicsControl | 同 Free，但有对齐目标位置/旋转 |
| `HumanBody` | PhysicsControl | 拖拽敌人尸体，控制指定骨骼 |
| `WeaponSnap` | Attach | 武器直接附加到手上，使用 `WeaponGrabOffsets` 定位（需要精确定位，不需要投掷） |
| `Custom` | 自定义函数 | 通过 `AGrabbeeObject` 的虚函数 `OnGrab()` / `OnRelease()` 实现 |
| `None` | 无实现 | 不可抓取或抓取被禁用 |

#### 为什么 Free/Snap 使用 PhysicsControl 而非 Attach？

参考 `BP_Breakable_01` 的旧方案：
- Attach 后物体无物理模拟，需要手动采样手柄速度（环形缓冲区 4 帧）
- 释放时手动设置物体速度，需要经验系数（如 1.5x）调参
- 手感不够自然

使用 PhysicsControl 的优势：
- 物体保持物理模拟，始终有真实的物理速度
- 释放时只需解除控制，无需手动设置速度
- 投掷手感更自然，无需调参

### 4.3 代码调用

- `GrabType` 作为 `AGrabbeeObject` 的属性
- `PlayerGrabHand.GrabObject()` 根据目标的 `GrabType` 选择不同的抓取方式
- `Custom` 类型通过 `AGrabbeeObject` 的虚函数 `OnGrab()` / `OnRelease()` 实现

---

## 五、抓取流程重构

### 5.1 GrabEvent 流程

```
GrabEvent
    │
    ├── 1. Validate
    │       检查 CanGrab、IsHolding 等状态
    │
    ├── 2. FindTarget (virtual, 子类实现)
    │       ├── 若 InBackPack == true
    │       │       → 从 InventoryComponent 获取弓或箭
    │       └── 若 InBackPack == false
    │               ├── [PC] 从摄像机中心发射射线检测
    │               └── [VR] 在手部周围进行球形检测
    │
    └── 3. GrabObject
            根据 Target.GrabType 执行对应抓取逻辑
            调用 Target.OnGrab(this)
```

### 5.2 ReleaseEvent 流程

设计思路同上，分为 Validate → 判断放入背包还是释放 → ReleaseObject 三步。

---

## 六、Gravity Gloves（隔空抓取）

Gravity Gloves 功能在 PC 和 VR 端逻辑不同，因此：
- `UPlayerGrabHand` 基类**不包含** Gravity Gloves 逻辑
- `UVRGrabHand` 实现 VR 端的 Gravity Gloves（角度检测、阻尼拉取）
- `UPCGrabHand` 实现 PC 端的 Gravity Gloves（待定）

### 6.1 VRGrabHand 的 Gravity Gloves

从旧设计迁移以下逻辑：
- `FindAngleClosestTarget()` - 在 Tick 中检测角度最近的可抓取物体
- `GravityGlovesTarget` - 当前锁定的远程目标
- `bIsGravityGlovesDragging` - 是否正在隔空拖拽
- 阻尼控制（EnableDamping / DisableDamping）

---

## 七、PhysicsControl 设计

### 7.1 组件归属

- `UPhysicsControlComponent` 放在 **Player 级别**（`ABasePlayer`）进行共享
- PC 和 VR 通过不同方式设置控制点：
  - **VR**：控制点在 MotionController 位置
  - **PC**：控制点由程序设置

### 7.2 使用场景

| GrabType | 使用 PhysicsControl |
|----------|---------------------|
| `Free` | ✓ 物体跟随手部（保持相对位置），保持物理模拟 |
| `Snap` | ✓ 物体跟随手部并对齐，保持物理模拟 |
| `HumanBody` | ✓ 拖拽敌人尸体，控制指定骨骼 |
| `WeaponSnap` | ✗ 使用 Attach |
| `Custom` | 由子类决定 |

### 7.3 PhysicsControl 参数配置

不同 GrabType 使用不同的 PhysicsControl 参数：

```cpp
// UPlayerGrabHand
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Physics")
FPhysicsControlSettings FreeGrabSettings;   // Free 类型的物理控制参数

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Physics")
FPhysicsControlSettings SnapGrabSettings;   // Snap 类型的物理控制参数

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Physics")
FPhysicsControlSettings HumanBodySettings;  // HumanBody 类型的物理控制参数
```

### 7.4 释放逻辑

使用 PhysicsControl 时，释放流程简化为：
1. 销毁/禁用 PhysicsControl
2. 物体自动保留当前物理速度（无需手动设置）

对比旧方案（Attach + 速度采样）：
```blueprint
// 旧方案：释放时需要手动设置速度
On Dropped:
    BaseStaticMesh.SetPhysicsLinearVelocity(GetVectorArrayAverage(RecentVelocities) * 1.5)
```

新方案无需此步骤。

---

## 八、双手交互

### 8.1 基本逻辑

`UPlayerGrabHand` 持有对另一只手的引用：
```cpp
UPROPERTY()
UPlayerGrabHand* OtherHand;
```

在 GrabEvent 中检查：如果目标正被另一只手持有，则先让另一只手释放。

### 8.2 特例：同一物体多部位抓取

以下情况允许两只手抓取同一物体的不同部位：

1. **弓**：一只手抓弓身，另一只手抓弓弦
2. **敌人尸体**：两只手抓取不同骨骼，分别创建 PhysicsControl

#### PC 与 VR 的差异

| 场景 | VR | PC |
|------|-----|-----|
| 弓 | 双手可分别抓取弓身和弓弦 | 待定 |
| 敌人尸体 | 双手可抓取不同部位 | 单手抓取固定点，抓取期间锁定（不能抓取其他物体） |

---

## 九、组件结构

### 9.1 BasePCPlayer

```
Root (CapsuleComponent)
├── FirstPersonCamera (CameraComponent)
├── LeftHand (UPCGrabHand)
└── RightHand (UPCGrabHand)
```

### 9.2 BaseVRPlayer

```
Root (CapsuleComponent)
└── VROrigin (SceneComponent)
    ├── Camera (CameraComponent)
    │   └── BackPackCollision (BoxComponent)
    ├── MotionControllerLeft (MotionSource=Left)
    │   └── LeftHand (UVRGrabHand)
    ├── MotionControllerRight (MotionSource=Right)
    │   └── RightHand (UVRGrabHand)
    ├── MotionControllerLeftAim (MotionSource=LeftAim)
    │   └── WidgetInteractionLeft
    └── MotionControllerRightAim (MotionSource=RightAim)
        └── WidgetInteractionRight
```

---

## 十、关联修改

### 10.1 InventoryComponent

将现有的 `AActor*` 引用改为 `AGrabbeeObject*`，使类型更符合语义。

---

## 十一、迁移对照表

| 原位置 | 内容 | 新位置 |
|--------|------|--------|
| BP_MyMotionController | Grabber 的抓取逻辑 | UPlayerGrabHand |
| BP_MyMotionController | Grabber 的属性（IsHolding, CanGrab 等） | UPlayerGrabHand |
| BP_MyMotionController | Gravity Gloves 逻辑 | UVRGrabHand |
| GrabComponent | Grabbee 的属性（GrabType 等） | AGrabbeeObject |
| GrabComponent | OnGrab/OnRelease 接口 | AGrabbeeObject |
| GrabComponent | 属于 Grabber 的逻辑（TryGrab 等） | UPlayerGrabHand |

---

## 待讨论

### 武器定位方案（弃用骨骼插槽）

旧设计中，WeaponSnap 类型使用手部骨骼网格体上的插槽定位武器。但 PC 模式没有骨骼网格体，需要新方案。

#### 新方案：抓取点放在 Hand 侧

由于目前只有弓和箭需要 WeaponSnap，将定位信息放在 `UPlayerGrabHand` 中更简洁。

##### 数据结构

```cpp
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    None,
    Bow,
    Arrow
};

// UPlayerGrabHand
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Weapon")
TMap<EWeaponType, FTransform> WeaponGrabOffsets;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
bool bIsRightHand = true;

// AGrabbeeWeapon（武器基类）
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
EWeaponType WeaponType;
```

##### 左右手配置

左右手的抓取姿态不同，因此**左右手分别配置** `WeaponGrabOffsets`：
- `UPCGrabHand` / `UVRGrabHand` 的蓝图子类中，根据 `bIsRightHand` 配置不同的 Offset

##### 抓取代码

```cpp
void UPlayerGrabHand::GrabWeaponSnap(AGrabbeeWeapon* Weapon)
{
    FTransform* Offset = WeaponGrabOffsets.Find(Weapon->WeaponType);
    if (Offset)
    {
        FTransform TargetTransform = GetComponentTransform() * (*Offset);
        Weapon->SetActorTransform(TargetTransform);
        Weapon->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
    }
}
```

##### 优点

1. 简单直接，武器种类少时清晰
2. 不依赖骨骼网格体，PC/VR 通用
3. 在蓝图中可视化配置

---

## 十二、PC 与 VR 操作差异

PC 和 VR 的操作逻辑差异较大，需要在各自的 GrabHand 子类中分别实现。

### 12.1 操作模式对比

| 方面 | PC | VR |
|------|-----|-----|
| 模式切换 | 十字键切换弓箭/徒手模式 | 无模式，物理动作 |
| 拾取方式 | 射线检测 + 按键 | 手部球形检测 + 握键 |
| 背包交互 | 自动（弓箭模式自动取弓） | 手伸到背后区域抓取 |
| 弓箭操作 | 程序化瞄准/拉弓 | 物理双手操作 |
| 丢弃方式 | 射线检测目标位置瞬移 | 松开握键自然掉落/投掷 |

### 12.2 PC 模式状态机

PC 端有明确的模式切换，使用状态机管理：

```cpp
UENUM(BlueprintType)
enum class EPCCombatMode : uint8
{
    Unarmed,    // 徒手模式
    BowArmed    // 弓箭模式
};
```

状态机放在 `ABasePCPlayer` 中：

```cpp
// ABasePCPlayer
UPROPERTY(BlueprintReadOnly, Category = "Combat")
EPCCombatMode CurrentCombatMode = EPCCombatMode::Unarmed;

UPROPERTY(BlueprintReadOnly, Category = "Combat")
bool bHasBowInInventory = false;  // 是否拥有弓（一旦拾取不会丢失）

UFUNCTION(BlueprintCallable, Category = "Combat")
void SwitchCombatMode(EPCCombatMode NewMode);
```

### 12.3 输入绑定

#### PC 输入（Enhanced Input）

| 输入 | 徒手模式 | 弓箭模式 |
|------|----------|----------|
| LT | 左手拾取/丢弃 | 开始瞄准（左手进入瞄准位置） |
| RT | 右手拾取/丢弃 | 拉弓/射箭 |
| 十字键↑ | 切换到弓箭模式 | - |
| 十字键↓ | 切换到徒手模式 | - |

```cpp
// ABasePCPlayer 输入处理
void ABasePCPlayer::HandleLeftTrigger(bool bPressed)
{
    if (CurrentCombatMode == EPCCombatMode::Unarmed)
    {
        if (bPressed) LeftHand->TryGrabOrRelease();
    }
    else // BowArmed
    {
        if (bPressed) StartAiming();
        else StopAiming();
    }
}

void ABasePCPlayer::HandleRightTrigger(bool bPressed)
{
    if (CurrentCombatMode == EPCCombatMode::Unarmed)
    {
        if (bPressed) RightHand->TryGrabOrRelease();
    }
    else // BowArmed
    {
        if (bPressed) StartDrawBow();
        else ReleaseBowString();
    }
}
```

#### VR 输入

| 输入 | 功能 |
|------|------|
| 左手握键 | 左手抓取/释放 |
| 右手握键 | 右手抓取/释放 |
| 手伸到背后 + 握键 | 从背包取物 |

VR 输入更简单，直接绑定到 GrabHand：

```cpp
// ABaseVRPlayer
void ABaseVRPlayer::HandleLeftGrip(bool bPressed)
{
    if (bPressed) LeftHand->TryGrab();
    else LeftHand->TryRelease();
}
```

### 12.4 PC 徒手模式详细逻辑

#### 拾取逻辑 (UPCGrabHand)

```cpp
void UPCGrabHand::TryGrabOrRelease()
{
    if (HeldObject)
    {
        // 手里有东西 → 丢弃到射线目标位置
        DropToRaycastTarget();
    }
    else
    {
        // 手里没东西 → 尝试拾取
        TryGrab();
    }
}

void UPCGrabHand::DropToRaycastTarget()
{
    FHitResult Hit;
    FVector Start = GetOwnerCamera()->GetComponentLocation();
    FVector End = Start + GetOwnerCamera()->GetForwardVector() * MaxDropDistance;
    
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
    {
        // 将物体瞬移到射线碰撞点
        HeldObject->SetActorLocation(Hit.Location);
    }
    
    ReleaseObject();
}
```

#### 射线检测 FindTarget

```cpp
// UPCGrabHand override
AGrabbeeObject* UPCGrabHand::FindTarget_Implementation()
{
    FHitResult Hit;
    FVector Start = GetOwnerCamera()->GetComponentLocation();
    FVector End = Start + GetOwnerCamera()->GetForwardVector() * MaxGrabDistance;
    
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, GrabTraceChannel))
    {
        return Cast<AGrabbeeObject>(Hit.GetActor());
    }
    return nullptr;
}
```

### 12.5 PC 弓箭模式详细逻辑

#### 模式切换

```cpp
void ABasePCPlayer::SwitchCombatMode(EPCCombatMode NewMode)
{
    if (NewMode == CurrentCombatMode) return;
    
    // 退出旧模式
    if (CurrentCombatMode == EPCCombatMode::BowArmed)
    {
        HideBow();
    }
    
    // 进入新模式
    CurrentCombatMode = NewMode;
    
    if (NewMode == EPCCombatMode::BowArmed && bHasBowInInventory)
    {
        ShowBow();  // 从背包取出弓，附加到左手
    }
}
```

#### 瞄准系统

```cpp
// ABasePCPlayer
UPROPERTY(EditAnywhere, Category = "Bow|Aiming")
FTransform AimingLeftHandTransform;  // 瞄准时左手的位置（相对于摄像机）

UPROPERTY(BlueprintReadOnly, Category = "Bow")
bool bIsAiming = false;

void ABasePCPlayer::StartAiming()
{
    bIsAiming = true;
    // 将左手平滑过渡到瞄准位置
    LeftHand->InterpToTransform(AimingLeftHandTransform);
}

void ABasePCPlayer::StopAiming()
{
    bIsAiming = false;
    // 左手回到默认位置
    LeftHand->InterpToDefaultTransform();
}
```

#### 程序化拉弓

```cpp
// ABasePCPlayer
UPROPERTY(EditAnywhere, Category = "Bow|Draw")
float MaxShootDistance = 5000.f;  // 最大射击距离

UPROPERTY(EditAnywhere, Category = "Bow|Draw")
float MaxDrawDistance = 50.f;  // 最大拉弓距离（右手相对于弓的距离）

UPROPERTY(BlueprintReadOnly, Category = "Bow")
bool bIsDrawingBow = false;

void ABasePCPlayer::StartDrawBow()
{
    if (!bIsAiming || !HasArrowInInventory()) 
    {
        PlayNoArrowSound();
        return;
    }
    
    bIsDrawingBow = true;
    
    // 1. 右手抓取弓弦
    RightHand->GrabBowString(CurrentBow);
    
    // 2. 生成箭并搭在弓弦上
    SpawnArrowOnBowString();
    
    // 3. 计算目标拉弓距离
    float TargetDistance = CalculateDrawDistance();
    
    // 4. 程序化拉弓（使用 Timeline 或 Interp）
    InterpDrawBow(TargetDistance);
}

float ABasePCPlayer::CalculateDrawDistance()
{
    // 射线检测目标位置
    FHitResult Hit;
    FVector Start = GetCamera()->GetComponentLocation();
    FVector End = Start + GetCamera()->GetForwardVector() * MaxShootDistance;
    
    float TargetDist = MaxShootDistance;
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
    {
        TargetDist = FMath::Min(Hit.Distance, MaxShootDistance);
    }
    
    // 根据目标距离计算需要的拉弓距离
    // （需要根据弓的物理参数调整映射关系）
    return FMath::GetMappedRangeValueClamped(
        FVector2D(0, MaxShootDistance),
        FVector2D(MinDrawDistance, MaxDrawDistance),
        TargetDist
    );
}

void ABasePCPlayer::ReleaseBowString()
{
    if (!bIsDrawingBow) return;
    
    bIsDrawingBow = false;
    
    // 释放弓弦，发射箭矢
    CurrentBow->ReleaseString();
    RightHand->ReleaseBowString();
    
    // 右手回到默认位置
    RightHand->InterpToDefaultTransform();
}
```

### 12.6 VR 弓箭操作

VR 端保持原有的物理操作方式，无需程序化：

1. **取弓**：手伸到背后 + 握键 → 从 InventoryComponent 取出弓
2. **取箭**：同上，弓取出后再取会取出箭
3. **拉弓**：一只手持弓，另一只手抓取弓弦
4. **射箭**：释放弓弦的手

#### 弓弦交互

```cpp
// ABow
UPROPERTY(EditAnywhere, Category = "Bow")
USphereComponent* BowStringGrabCollision;  // 弓弦抓取区域

// 弓弦被抓取时
void ABow::OnStringGrabbed(UPlayerGrabHand* Hand)
{
    StringHoldingHand = Hand;
    bIsStringHeld = true;
}

// 弓弦被释放时
void ABow::OnStringReleased()
{
    if (bIsStringHeld && CurrentDrawAmount > MinDrawToFire)
    {
        FireArrow();
    }
    StringHoldingHand = nullptr;
    bIsStringHeld = false;
}
```

### 12.7 弓的特殊处理

#### 首次拾取

弓一旦拾取就永久持有，通过标志位记录：

```cpp
// ABasePCPlayer / ABaseVRPlayer
void OnBowFirstPickedUp(ABow* Bow)
{
    bHasBowInInventory = true;
    InventoryComponent->StoreBow(Bow);
    
    // PC: 可以选择立即切换到弓箭模式
    // VR: 弓被放下时自动存入背包
}
```

#### PC 释放弓

PC 弓箭模式下，弓无法被丢弃，只能切换回徒手模式（弓自动收回背包）。

#### VR 释放弓

VR 释放弓时，不是真的丢弃，而是存入背包：

```cpp
// UVRGrabHand override
void UVRGrabHand::ReleaseObject()
{
    if (ABow* Bow = Cast<ABow>(HeldObject))
    {
        // 弓不能被丢弃，存入背包
        GetInventoryComponent()->StoreBow(Bow);
        HeldObject = nullptr;
        return;
    }
    
    // 其他物体正常释放
    Super::ReleaseObject();
}
```

### 12.8 GrabHand 子类职责汇总

#### UPCGrabHand

| 职责 | 说明 |
|------|------|
| FindTarget | 射线检测 |
| TryGrabOrRelease | 根据手中是否有物体决定拾取或丢弃 |
| DropToRaycastTarget | 丢弃到射线目标位置 |
| InterpToTransform | 程序化移动手部位置 |
| GrabBowString | 程序化抓取弓弦 |
| ReleaseBowString | 程序化释放弓弦 |

#### UVRGrabHand

| 职责 | 说明 |
|------|------|
| FindTarget | 球形检测 |
| TryGrab / TryRelease | 物理抓取/释放 |
| CheckBackpackArea | 检测手是否在背包区域 |
| Gravity Gloves | 隔空抓取逻辑 |
| 弓特殊处理 | 释放弓时自动存入背包 |

---

## 十三、弓箭系统集成

### 13.1 弓类设计

```cpp
UCLASS()
class ABow : public AGrabbeeWeapon
{
    // 弓弦相关
    UPROPERTY()
    USphereComponent* StringGrabCollision;
    
    UPROPERTY()
    UPlayerGrabHand* StringHoldingHand;
    
    UPROPERTY()
    AArrow* NockedArrow;  // 搭在弦上的箭
    
    UPROPERTY(BlueprintReadOnly)
    float CurrentDrawAmount;  // 当前拉弓程度 [0, 1]
    
    // 接口
    void OnStringGrabbed(UPlayerGrabHand* Hand);
    void OnStringReleased();
    void NockArrow(AArrow* Arrow);
    void UpdateArrowTransform();  // Tick 中更新箭的位置/旋转
    void FireArrow();
};
```

### 13.2 箭类设计

```cpp
UCLASS()
class AArrow : public AGrabbeeWeapon
{
    UPROPERTY(EditAnywhere)
    EArrowState CurrentState;  // Idle, Nocked, Flying, Stuck
    
    void OnNocked(ABow* Bow);
    void OnFired(FVector Velocity);
    void OnHit(FHitResult Hit);
};
```

### 13.3 弓箭与抓取系统的交互

| 操作 | 触发 | 结果 |
|------|------|------|
| 抓取弓身 | GrabHand.GrabObject(Bow) | WeaponSnap 附加到手上 |
| 抓取弓弦 | GrabHand.GrabObject(Bow.StringCollision) | Bow.OnStringGrabbed(Hand) |
| 释放弓弦 | GrabHand.ReleaseObject() | Bow.OnStringReleased() → FireArrow |
| 释放弓身(VR) | VRGrabHand.ReleaseObject() | 存入背包 |

