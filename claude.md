# Claude Code 项目记忆

## 项目基本信息

**项目类型**：VR潜行战斗游戏（支持PC和VR双平台）
**引擎**：Unreal Engine
**开发语言**：C++ + 蓝图
**开发阶段**：重构中（从蓝图逻辑迁移到C++）
**协作工具**：BP2AI插件（一键将蓝图转换为md文档）

## 项目文件结构

```
Source/VRTest/
├── Public/
│   ├── Effect/          # 效果/伤害系统
│   ├── Game/            # 游戏核心逻辑（玩家、抓取）
│   └── Grabbee/         # 可抓取物体
├── Private/
│   ├── Effect/
│   ├── Game/
│   └── Grabbee/

Docs/
├── GamePlayDesign.txt   # 游戏玩法设计文档
└── Refactor/            # 重构设计文档
    ├── Grab.md          # 抓取系统重构方案
    ├── Bow&Ops.md       # 弓箭操作设计
    └── Effectable&Character.md  # 效果和角色设计

BP2AIExport/
└── Program/             # 重构后的蓝图导出文档
```

## 已完成的重构模块

### 1. Effect 模块（效果/伤害系统）

#### 核心接口：IEffectable
- 定义 `ApplyEffect(const FEffect& Effect)` 函数
- 所有可受影响的对象实现此接口

#### 结构体：FEffect
```cpp
struct FEffect {
    TArray<EEffectType> EffectTypes;  // 效果类型数组
    float Amount;                      // 效果量（伤害值等）
    float Duration;                    // 持续时间
    AActor* Causer;                    // 效果来源Actor
    ABaseCharacter* Instigator;        // 效果发起者
};
```

#### 枚举：EEffectType
- Arrow（箭矢）
- Smash（钝击）
- Melee（近战）
- Fire（火焰）
- Stasis（定身）

#### 组件：UAliveComponent
- `float MaxHP` - 最大生命值
- `float HP` - 当前生命值
- `FOnDead OnDead` - 死亡委托
- `DecreaseHP()` / `IncreaseHP()` / `SetHP()` / `GetHP()` - 生命值管理

#### 组件：UFallDamageComponent
- 依赖 AliveComponent
- 监听 Character 的 OnLanded 事件
- 根据Z轴速度计算摔落伤害
- 可配置摔落阈值和伤害系数

#### 组件：UAutoRecoverComponent
- 依赖 AliveComponent
- 在 Tick 中持续恢复生命值
- 可配置回血速度

### 2. Game 模块（角色和玩家）

#### 类层次结构

```
ACharacter
└── ABaseCharacter (实现 IEffectable)
    ├── BaseEnemy
    └── ABasePlayer
        ├── ABasePCPlayer
        └── ABaseVRPlayer
```

#### ABaseCharacter
- 包含 `UAliveComponent*`
- 包含 `UInventoryComponent*`
- 实现 IEffectable 接口：
  - `ApplyEffect_Implementation()` - 遍历 EffectTypes 调用对应的 TakeEffect
  - `TakeArrowEffect()` / `TakeSmashEffect()` / `TakeMeleeEffect()` / `TakeFireEffect()` / `TakeStasisEffect()` - 各类型效果处理函数（虚函数，子类可重写）
- `GetTrackOrigin()` - 返回AI视觉刺激点（默认返回CapsuleComponent）
- `OnDeath()` - 死亡事件（蓝图可重写）

#### ABasePlayer
- 添加组件：
  - `UFallDamageComponent*`
  - `UAutoRecoverComponent*`
  - `UPhysicsControlComponent*` - 用于抓取系统的物理控制
- 弓相关：
  - `bool bHasBow` - 是否获得弓（永久）
  - `bool bIsBowArmed` - 是否处于弓箭模式
  - `AGrabbeeWeapon* CurrentBow` - 当前的弓
  - `OnBowFirstPickedUp()` - 首次获得弓
  - `SetBowArmed(bool)` / `ToggleBowArmed()` - 切换弓箭模式

##### ABasePCPlayer
- 添加 `UPCGrabHand* LeftHand` / `UPCGrabHand* RightHand` 组件
- **目标检测**：
  - `AActor* TargetedObject` - 当前瞄准的可抓取物体（实现 IGrabbable）
  - `FOnGrabTargetChanged OnGrabTargetChanged` - 瞄准目标变化委托
  - `UpdateTargetDetection()` - 每帧执行射线检测（仅在徒手模式且双手空闲时），使用接口验证
  - `PerformLineTrace()` - 执行射线检测
  - `OnHandGrabbedObject()` - 监听手的抓取事件，立即清空瞄准目标
- 实现各自平台的输入处理

### 3. 抓取系统（Grab System）

#### 设计原则
- **职责分离**：Grabber（抓取者）和 Grabbee（被抓取物）完全分离
- **接口驱动**：Grabber 侧完全面向 `IGrabbable` 接口编程
- **平台解耦**：PC 和 VR 通过继承实现差异化
- **C++化**：核心逻辑从蓝图迁移到C++

#### Grabber 侧（手部组件）

```
USceneComponent
└── UPlayerGrabHand (抓取基类)
    ├── UPCGrabHand (PC模式)
    └── UVRGrabHand (VR模式)
```

##### UPlayerGrabHand（基类）
**职责**：
- 管理抓取状态（`bIsHolding`, `HeldActor`, `HeldGrabType`）
- 完全面向 IGrabbable 接口编程
- 实现抓取/释放核心逻辑
- 根据 GrabType 选择抓取方式
- 管理 PhysicsControl

**关键属性**：
- `bool bIsRightHand` - 左右手标识
- `UPlayerGrabHand* OtherHand` - 另一只手的引用
- `TMap<EWeaponType, FTransform> WeaponGrabOffsets` - 武器抓取偏移
- `AActor* HeldActor` - 当前持有的 Actor（实现 IGrabbable）
- `EGrabType HeldGrabType` - 当前持有物体的抓取类型（缓存）
- `FName CurrentControlName` - 当前 PhysicsControl 句柄
- `FName GrabbedBoneName` - 当前抓取的骨骼名（HumanBody 类型用）
- `FTransform GrabOffset` - Free/Snap 抓取时记录的相对变换（物体相对于手）

**核心接口**：
- `TryGrab(bool bFromBackpack)` - 尝试抓取
- `TryRelease(bool bToBackpack)` - 尝试释放
- `FindTarget(bool bFromBackpack, FName& OutBoneName)` - 查找目标（虚函数，子类实现）
  - 返回 `AActor*`（实现 IGrabbable）
  - 输出参数 OutBoneName：命中的骨骼名（如果目标是骨骼网格体）
- `GrabObject(AActor*, FName BoneName)` - 执行抓取
  - 内部通过 `Cast<IGrabbable>` 获取接口
  - 使用 `IGrabbable::Execute_*` 调用接口方法
  - 支持双手抓取的物体不会强制释放另一只手
- `ReleaseObject()` - 执行释放

**抓取实现函数**（接收 IGrabbable* 和 AActor*）：
- `GrabFree()` - Free类型抓取（PhysicsControl）
- `GrabSnap()` - Snap类型抓取（PhysicsControl + 对齐）
- `GrabWeaponSnap()` - 武器抓取（Attach）
- `GrabHumanBody()` - 人体拖拽（PhysicsControl）

**PhysicsControl 更新**：
- 在 Tick 中更新 PhysicsControl 目标位置
- Free：保持抓取时的相对变换（`HandTransform * GrabOffset`）
- Snap：跟随预设的对齐变换（`HandTransform * SnapOffset`）
- HumanBody：跟随手部位置

**抓取时的相对变换记录**：
- `GrabFree()`：计算并记录 `GrabOffset = ObjectTransform.GetRelativeTransform(HandTransform)`
- `GrabSnap()`：记录 `GrabOffset = Target->SnapOffset`
- Tick 中统一使用 `HandTransform * GrabOffset` 计算目标位置

##### UPCGrabHand（PC模式）
**特性**：
- 使用**射线检测**寻找目标（复用 Player 的检测结果）
- 支持**程序化手部位置**（InterpToTransform）
- 丢弃物体时瞬移到射线碰撞点

**关键属性**：
- `float MaxDropDistance` - 丢弃射线最大距离
- `FTransform DefaultRelativeTransform` - 默认手部位置（相对摄像机）
- `float HandInterpSpeed` - 手部插值速度

**核心函数**：
- `FindTarget()` - 优先从背包取物，否则使用 Player 的 TargetedObject
- `TryGrabOrRelease()` - 根据手中是否有物体决定抓取或丢弃
- `DropToRaycastTarget()` - 丢弃到射线目标位置
- `InterpToTransform()` / `InterpToDefaultTransform()` - 程序化移动手部
- `GetOwnerPCPlayer()` - 获取所属 PC 玩家（用于访问 TargetedObject）

**性能优化**：
- 不再每帧执行射线检测，直接使用 Player 的检测结果
- 避免双手重复检测，提升性能

##### UVRGrabHand（VR模式）
**特性**：
- 使用**球形检测**寻找近距离目标
- 支持 **Gravity Gloves**（隔空抓取）
- 背包区域检测

**关键属性**：
- `float GrabSphereRadius` - 球形检测半径
- `UBoxComponent* BackpackCollision` - 背包碰撞区域引用

**Gravity Gloves 属性**：
- `bool bEnableGravityGloves` - 是否启用
- `float GravityGlovesDistance` - 检测距离
- `float GravityGlovesAngle` - 锁定角度
- `float PullBackThreshold` - 向后拉动速度阈值
- `AActor* GravityGlovesTarget` - 当前选中的远程目标（实现 IGrabbable）
- `bool bIsVirtualGrabbing` - 是否正在虚拟抓取
- `FVector HandVelocity` - 手部速度

**核心函数**：
- `TryGrab(bool bFromBackpack)` - 重写基类，若未指定参数则自动检测背包区域
- `FindTarget()` - 优先球形检测，其次 GravityGlovesTarget
- `FindAngleClosestTarget()` - 查找角度最近的目标（返回 AActor*）
- `VirtualGrab(AActor*)` - 虚拟抓取（设置状态但不附加）
- `VirtualRelease(bool bLaunch)` - 虚拟释放（可选发射物体）
- `UpdateGravityGloves()` - 每帧更新 Gravity Gloves 逻辑
- `CheckPullBackGesture()` - 检测向后拉动手势
- `IsInBackpackArea()` - 检查手是否在背包区域

**Gravity Gloves 工作流程**：
1. 未抓取时：持续 `FindAngleClosestTarget()` 寻找目标，更新 `GravityGlovesTarget` 并调用 `OnGrabSelected()` / `OnGrabDeselected()`（通过接口）
2. 玩家按下 Grip：
   - 近距离目标：正常抓取
   - 远距离 GravityGlovesTarget：`VirtualGrab()` 虚拟抓取
3. 虚拟抓取状态下：检测向后拉动手势，触发时调用 `VirtualRelease(true)` 发射物体到手部

#### Grabbee 侧（可抓取物体）

**接口架构（IGrabbable）**

```
IGrabbable (接口)
├── AGrabbeeObject (可抓取物体基类，实现IGrabbable)
│   └── AGrabbeeWeapon (武器基类)
│       ├── ABow (弓)
│       └── AArrow (箭)
└── ABaseEnemy (敌人基类，实现IGrabbable)
```

##### IGrabbable 接口
**位置**：`Source/VRTest/Public/Grab/IGrabbable.h`

**职责**：
- 定义所有可抓取物体的统一接口
- Grabber 侧完全面向此接口编程，不需要知道具体实现类

**查询方法**：
- `GetGrabType()` - 获取抓取类型
- `GetGrabPrimitive()` - 获取用于物理控制的 PrimitiveComponent
- `CanBeGrabbedBy(const UPlayerGrabHand*)` - 检查是否可被指定手抓取
- `GetSnapOffset()` - 获取 Snap 类型的目标变换
- `SupportsDualHandGrab()` - 是否支持双手同时抓取

**状态回调**：
- `OnGrabbed(UPlayerGrabHand*)` - 被抓取时调用
- `OnReleased(UPlayerGrabHand*)` - 被释放时调用
- `OnGrabSelected()` - 被选中时调用（Gravity Gloves 瞄准）
- `OnGrabDeselected()` - 取消选中时调用

##### AGrabbeeObject（实现 IGrabbable）
**职责**：
- 可抓取物品的基类（非敌人类型）
- 实现 IGrabbable 接口的所有方法

**关键属性**：
- `UStaticMeshComponent* MeshComponent` - 根组件
- `EGrabType GrabType` - 抓取类型
- `bool bCanGrab` - 是否可抓取
- `FTransform SnapOffset` - Snap类型的对齐偏移
- `bool bSupportsDualHandGrab` - 是否支持双手同时抓取（HumanBody 默认支持）
- `bool bIsHeld` - 是否被抓取
- `UPlayerGrabHand* HoldingHand` - 抓取此物体的手（主手）
- `TSet<UPlayerGrabHand*> ControllingHands` - 当前控制此物体的所有手（双手抓取用）
- `bool bIsSelected` - 是否被选中（Gravity Gloves）

**IGrabbable 实现**：
- `GetGrabType_Implementation()` - 返回 GrabType 属性
- `GetGrabPrimitive_Implementation()` - 返回 MeshComponent
- `CanBeGrabbedBy_Implementation()` - 检查 bCanGrab 和双手抓取逻辑
- `GetSnapOffset_Implementation()` - 返回 SnapOffset 属性
- `SupportsDualHandGrab_Implementation()` - 返回 bSupportsDualHandGrab
- `OnGrabbed_Implementation()` - 更新 bIsHeld、HoldingHand、ControllingHands
- `OnReleased_Implementation()` - 清理抓取状态
- `OnGrabSelected_Implementation()` / `OnGrabDeselected_Implementation()` - 高亮效果

**其他函数**：
- `LaunchTowards(FVector, float)` - 向目标位置发射（Gravity Gloves）
- `SetSimulatePhysics(bool)` - 设置物理模拟
- `ForceRelease()` - 强制从当前手释放

##### ABaseEnemy（实现 IGrabbable）
**职责**：
- 敌人基类，死亡后尸体可被拖拽
- 实现 IGrabbable 接口（HumanBody 类型）

**IGrabbable 实现**：
- `GetGrabType_Implementation()` - 返回 `EGrabType::HumanBody`
- `GetGrabPrimitive_Implementation()` - 返回 `GetMesh()`（骨骼网格体）
- `CanBeGrabbedBy_Implementation()` - 返回 `bIsDead`（只有死亡敌人可拖拽）
- `GetSnapOffset_Implementation()` - 返回 `FTransform::Identity`
- `SupportsDualHandGrab_Implementation()` - 返回 `true`
- `OnGrabbed_Implementation()` / `OnReleased_Implementation()` / `OnGrabSelected_Implementation()` / `OnGrabDeselected_Implementation()` - 空实现（尸体不需要状态管理）

##### AGrabbeeWeapon
- 添加 `EWeaponType WeaponType` 属性
- 用于 WeaponSnap 抓取

##### AArrow（箭）
**职责**：可发射的投射物，支持状态切换和伤害

**组件**：
- `UProjectileMovementComponent* ProjectileMovement` - 投射物移动
- `UBoxComponent* ArrowTipCollision` - 箭头碰撞检测
- `UNiagaraComponent* TrailEffect` - 飞行轨迹
- `UParticleSystemComponent* FireEffect` - 火焰效果

**状态（EArrowState）**：
- `Idle`：闲置，启用物理，可抓取
- `Nocked`：搭在弓弦上，禁用物理，跟随弓弦位置
- `Flying`：飞行中，使用 ProjectileMovement
- `Stuck`：插在目标上，附着到命中组件

**关键属性**：
- `float ArrowDamage` - 箭伤害值
- `float OnFireDuration` - 着火持续时间
- `bool bOnFire` - 是否着火
- `ABow* NockedBow` - 当前搭载此箭的弓
- `ABaseCharacter* ArrowInstigator` - 发射者

**核心函数**：
- `EnterIdleState()` - 进入闲置状态
- `EnterNockedState(ABow*)` - 进入搭弦状态
- `EnterFlyingState(float LaunchSpeed)` - 进入飞行状态
- `EnterStuckState(USceneComponent*, FName)` - 进入插入状态
- `CatchFire()` / `Extinguish()` - 火焰效果
- 实现 `IEffectable` 接口接收火焰效果

##### ABow（弓）
**职责**：管理弓弦、箭的搭载和发射

**组件**：
- `UStaticMeshComponent* StringMesh` - 弓弦网格体
- `UBoxComponent* StringCollision` - 弓弦碰撞区域
- `USceneComponent* BowFrontPosition` - 弓前端位置
- `USceneComponent* StringRestPosition` - 弓弦默认位置
- `UNiagaraComponent* TrajectoryPreview` - 轨迹预览
- `UAudioComponent* StringAudio / FireAudio` - 音效

**关键属性**：
- `float MaxPullDistance` - 最大拉弦距离
- `float FiringSpeedMultiplier` - 发射速度系数
- `bool bBodyHeld / bStringHeld` - 弓身/弓弦抓取状态
- `UPlayerGrabHand* BodyHoldingHand / StringHoldingHand` - 持有的手
- `AArrow* NockedArrow` - 当前搭载的箭
- `FVector CurrentGrabSpot` - 当前弓弦位置
- `float CurrentPullLength` - 当前拉弦长度
- `UMaterialInstanceDynamic* StringMID` - 弓弦动态材质
- `TSubclassOf<AArrow> ArrowClass` - 箭类（用于生成）

**核心函数**：
- `StartPullingString(UPlayerGrabHand*)` - 开始拉弦
- `ReleaseString()` - 释放弓弦（发射）
- `NockArrow(AArrow*)` - 搭箭
- `UnnockArrow()` - 取消搭箭
- `SpawnAndNockArrow()` - 生成并搭箭（PC模式）
- `FireArrow()` - 发射箭
- `SetPullAmount(float)` - 程序化设置拉伸量（PC模式）
- `CalculateFiringSpeed()` - 计算发射速度
- `UpdateTrajectoryPreview()` - 更新轨迹预览

**委托**：
- `FOnArrowFired OnArrowFired` - 箭发射时触发
- `FOnArrowNocked OnArrowNocked` - 箭搭上弓弦时触发

#### 抓取类型（EGrabType）

| GrabType | 实现方式 | 说明 |
|----------|---------|------|
| **Free** | PhysicsControl | 物体保持相对位置，物理跟随手部 |
| **Snap** | PhysicsControl | 物体对齐到目标位置/旋转，物理跟随 |
| **WeaponSnap** | Attach | 武器直接附加到手上，使用 WeaponGrabOffsets 定位 |
| **HumanBody** | PhysicsControl | 拖拽尸体，控制指定骨骼，支持双手同时抓取 |
| **Custom** | 自定义 | 通过 OnGrabbed/OnReleased 实现自定义逻辑 |
| **None** | 无 | 不可抓取 |

**HumanBody 双手抓取特性**：
- 每只手有独立的 PhysicsControl，控制不同骨骼
- VR：使用球形追踪（SphereTrace）获取命中的骨骼名
- PC：使用射线检测（LineTrace）获取命中的骨骼名
- 如果目标不是骨骼网格体，则忽略骨骼参数
- 控制名格式：`DragBody_[Left/Right]_[物体名]`

**为什么使用 PhysicsControl 而非 Attach？**
- 物体保持物理模拟，始终有真实的物理速度
- 释放时无需手动设置速度（旧方案需要速度采样+经验系数）
- 投掷手感更自然

#### 武器类型（EWeaponType）
- None
- Bow（弓）
- Arrow（箭）

#### 箭的状态（EArrowState）
- Idle（闲置，可抓取）
- Nocked（搭在弓弦上）
- Flying（飞行中）
- Stuck（插在目标上）

### 5. 弓箭系统工作流程

#### VR 模式
1. 玩家一只手抓弓（`ABow::OnGrabbed` → `bBodyHeld = true`）
2. 另一只手抓箭，靠近弓弦碰撞区域
3. 箭进入弓弦区域，手释放箭 → 自动搭箭（`ABow::NockArrow`）
4. 手抓弓弦（`ABow::StartPullingString`）
5. 拉动弓弦 → 弓弦跟随手部位置
6. 释放弓弦 → 发射箭（`ABow::ReleaseString` → `ABow::FireArrow`）

#### PC 模式
1. 玩家切换到弓箭模式（`SetBowArmed(true)`）
2. 弓自动生成并附着到左手
3. 按住瞄准键 → 生成箭并搭载（`ABow::SpawnAndNockArrow`）
4. 拉动控制（`ABow::SetPullAmount(0.0-1.0)`）
5. 释放瞄准键 → 发射箭

### 6. 库存系统（Inventory）

#### UInventoryComponent
- 属于 ABaseCharacter
- 管理背包中的物品
- 当前主要管理箭的存储/取出

**接口**（推测）：
- `TryStoreArrow()` - 存储箭到背包
- `TryRetrieveArrow(FTransform SpawnTransform)` - 从背包取出箭
- `IsArrowFull()` - 背包箭是否已满
- `StoreBow(AGrabbeeWeapon*)` - 存储弓

## PC 端目标检测架构

### 设计目标
避免双手重复射线检测，提升性能。

### 实现方案
**检测层级**：BasePCPlayer（每帧一次） → PCGrabHand（复用结果）

### BasePCPlayer 职责
1. **每帧检测**：在 Tick 中调用 `UpdateTargetDetection()`
2. **检测条件**：仅在徒手模式且双手空闲时执行
3. **接口验证**：通过 `Cast<IGrabbable>` 和 `IGrabbable::Execute_CanBeGrabbedBy` 验证目标
4. **状态管理**：
   - 记录 `AActor* TargetedObject`（当前瞄准的物体，实现 IGrabbable）
   - 触发 `OnGrabTargetChanged` 委托（UI 可绑定）
   - 通过接口调用 `OnGrabSelected()` / `OnGrabDeselected()`
5. **抓取同步**：监听双手的 `OnObjectGrabbed` 委托，抓取时立即清空 `TargetedObject`

### PCGrabHand 职责
1. **复用检测结果**：`FindTarget()` 直接返回 `GetOwnerPCPlayer()->TargetedObject`
2. **丢弃检测**：仅在 `DropToRaycastTarget()` 时执行射线检测
3. **不再有**：每帧检测逻辑、TargetedObject 成员变量、OnGrabTargetChanged 委托

### 性能优化
- 每帧只执行 1 次射线检测（而非 2 次）
- 左右手共享检测结果
- 检测逻辑集中管理，易于维护

| 方面 | PC | VR |
|------|-----|-----|
| **模式切换** | 十字键切换弓箭/徒手模式 | 无模式，物理动作 |
| **拾取方式** | 射线检测 + 按键 | 手部球形检测 + 握键 |
| **背包交互** | 自动（弓箭模式自动取弓） | 手伸到背后区域抓取 |
| **弓箭操作** | 二次抓取：LT瞄准+RT抓弦拉弓 | 物理双手操作 |
| **丢弃方式** | 射线检测目标位置瞬移 | 松开握键自然掉落/投掷 |
| **Gravity Gloves** | 待定 | 已实现 |

### PC 模式状态机（设计中，未完全实现）

```cpp
enum class EPCCombatMode : uint8 {
    Unarmed,    // 徒手模式
    BowArmed    // 弓箭模式
};
```

**输入绑定**：
- 徒手模式：LT/RT = 左右手拾取/丢弃
- 弓箭模式：LT = 开始瞄准（生成并搭箭），RT = 抓弓弦拉弓/释放发射
- 十字键↑/↓ = 切换模式

### PC 弓箭模式详细流程
1. **切换到弓箭模式** (`SetBowArmed(true)`)
   - 生成弓 Actor
   - 左手抓弓身（第一次抓取）
2. **开始瞄准** (`StartAiming`, LT按下)
   - 左手移动到瞄准位置
   - 检查箭数量，消耗一支箭
   - 弓生成箭并搭载 (`SpawnAndNockArrow`)
   - 弓标记为 PC 模式 (`bIsPCMode = true`)
3. **开始拉弓** (`StartDrawBow`, RT按下)
   - 右手抓弓弦（第二次抓取，弓的自定义逻辑）
   - 弓弦开始拉动
4. **拉弓过程** (Tick)
   - 右手位置驱动弓弦材质参数 (`UpdateStringFromHandPosition`)
   - 弓弦顶点动画随右手移动
5. **发射** (`ReleaseBowString`, RT松开)
   - 弓释放弓弦 (`ReleaseString`)
   - 箭发射
   - 右手回到默认位置
6. **停止瞄准** (`StopAiming`, LT松开)
   - 如果未发射，销毁箭并退还到背包
   - 左手回到默认位置

## 当前进度

### 已完成 ✓
1. Effect 模块完全实现
2. Game 模块基础架构完成
3. 抓取系统核心逻辑完成：
   - PlayerGrabHand 基类
   - PCGrabHand（射线检测、程序化手部）
   - VRGrabHand（球形检测、Gravity Gloves）
   - **PC端目标检测优化**：目标检测逻辑提升到 BasePCPlayer 层面，每帧只检测一次，避免双手重复检测
   - **HumanBody 双手抓取**：支持两只手同时抓取同一物体（如敌人身体），每只手有独立的 PhysicsControl 控制不同骨骼
4. Grabbee 物体基类完成
5. PhysicsControl 集成完成
6. **Free/Snap 抓取修复**：修复了相对变换计算错误，现在物体能正确保持抓取时的相对位置
7. **弓箭系统完整实现**：
   - ABow 和 AArrow 类
   - PC 模式二次抓取（左手弓身+右手弓弦）
   - 弓弦材质参数由右手位置驱动（顶点动画）
   - 箭生成在 StartAiming 时机

### 待实现
1. VR 弓弦交互集成（与 VRGrabHand 联动）
2. 特殊能力系统（鹰眼、定身术、隐身术、护盾术等）
3. 敌人AI系统
4. 场景互动与解谜

### 最近修改
- ✓ **PC 弓箭流程重构**：箭生成移到 StartAiming，拉弓改为右手抓弓弦
- ✓ **弓支持二次抓取**：ABow 的 CanBeGrabbedBy/OnGrabbed 支持弓身+弓弦分开抓取
- ✓ **弓弦材质驱动**：添加 UpdateStringFromHandPosition，由右手位置驱动顶点动画
- ✓ **弓箭系统实现**：完成 ABow 和 AArrow 类，支持状态机、发射、伤害、火焰效果等
- ✓ **VR 背包检查重构**：将背包区域检查逻辑从 `ABaseVRPlayer::HandleGrip` 迁移到 `UVRGrabHand::TryGrab`

## 重要设计决策

1. **职责分离**：Grabber 和 Grabbee 彻底分离，避免职责混淆
2. **平台解耦**：PC 和 VR 共用基类，各自实现差异化逻辑
3. **PhysicsControl 优于 Attach**：投掷手感更自然，无需手动速度采样
4. **武器定位放在 Hand 侧**：WeaponGrabOffsets 在 PlayerGrabHand 中配置，而非物体上的插槽
5. **Gravity Gloves 虚拟抓取**：使用虚拟抓取状态避免物体真正附加，直到发射到手部

## 代码风格

- 使用 UE 命名规范（U前缀Component，A前缀Actor，E前缀Enum，F前缀Struct）
- 虚函数使用 `_Implementation` 后缀
- 蓝图可调用函数标记 `BlueprintCallable`
- 蓝图可重写函数使用 `BlueprintNativeEvent`
- 分段注释标记（`// ==================== 配置 ====================`）

## 注意事项

1. **蓝图导出文档可能不完全准确**：蓝图编译通过但导出文档可能有误，需要参考实际蓝图
2. **弓的特殊处理**：弓一旦获得就永久持有，不能真正丢弃
3. **背包交互差异**：VR需要物理检测手部在背包区域，PC是自动/程序化的
4. **PhysicsControl 在 Player 级别共享**：所有抓取使用同一个 PhysicsControlComponent

## 协作模式

- **先前协作者**：GitHub Copilot（额度已满）
- **当前协作者**：Claude Code
- **协作方式**：阅读源码和设计文档，理解项目后协同开发
- **任务导向**：先完成核心系统，再打磨细节

---

**最后更新**：2025-01-15
**AI 状态**：已完成 HumanBody 双手抓取支持
**最近工作**：
1. GrabbeeObject：添加 `bSupportsDualHandGrab` 和 `ControllingHands` 支持多手抓取
2. PlayerGrabHand：FindTarget 和 GrabObject 支持骨骼名参数
3. VRGrabHand：新增 `PerformSphereTrace` 使用 SphereTraceSingle 获取骨骼名
4. BasePCPlayer：射线检测保存 `TargetedBoneName`
5. GrabHumanBody：每只手创建独立的 PhysicsControl，使用独立控制名和骨骼名
