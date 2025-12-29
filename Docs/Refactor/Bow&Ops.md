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

