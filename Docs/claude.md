# Claude Code 项目记忆

请AI大模型在维护这个记忆文件的时候保持精简，抓住要点而不是冗长描述。

## 项目基本信息

**项目类型**：VR潜行战斗游戏（支持PC和VR双平台）
**引擎**：Unreal Engine 5.4
**开发语言**：C++ + 蓝图
**开发阶段**：重构中（从蓝图逻辑迁移到C++）
**协作工具**：BP2AI插件（将蓝图转换为md文档）

## 项目文件结构

```
Source/VRTest/
├── Public/
│   ├── Effect/          # 效果/伤害系统
│   ├── Game/            # 游戏核心逻辑（玩家、角色、库存）
│   ├── Grabber/         # 抓取系统（GrabHand、接口、类型）
│   └── Grabbee/         # 可抓取物体
├── Private/
│   ├── Game/            # 玩家、角色实现
│   ├── Grabber/         # GrabHand实现
│   └── Grabbee/         # 可抓取物体实现

Docs/Refactor/           # 重构设计文档
BP2AIExport/Program/     # 蓝图导出文档
```

### Grabber 文件夹内容（2026-01-06重构）
```
Public/Grabber/
├── GrabTypes.h          # EGrabType、EWeaponType 枚举
├── IGrabbable.h         # 可抓取接口
├── PlayerGrabHand.h     # 抓取手基类
├── PCGrabHand.h         # PC模式抓取手
└── VRGrabHand.h         # VR模式抓取手

Private/Grabber/
├── PlayerGrabHand.cpp
├── PCGrabHand.cpp
└── VRGrabHand.cpp
```

## 已完成的重构模块

### 1. Effect 模块（效果/伤害系统）

- **IEffectable 接口**：`ApplyEffect(const FEffect& Effect)`
- **FEffect 结构体**：EffectTypes数组、Amount、Duration、Causer、Instigator
- **EEffectType 枚举**：Arrow、Smash、Melee、Fire、Stasis
- **UAliveComponent**：MaxHP、HP、OnDead委托、生命值管理
- **UFallDamageComponent**：监听OnLanded计算摔落伤害
- **UAutoRecoverComponent**：Tick中持续回血

### 2. Game 模块（角色和玩家）

#### 类层次结构
```
ACharacter
└── ABaseCharacter (实现 IEffectable)
    ├── ABaseEnemy (实现 IGrabbable)
    └── ABasePlayer
        ├── ABasePCPlayer
        └── ABaseVRPlayer
```

#### ABaseCharacter
- 包含 UAliveComponent、UInventoryComponent
- 实现 IEffectable 接口，根据 EffectType 分发到对应处理函数
- `GetTrackOrigin()` - AI视觉刺激点
- `OnDeath()` - 死亡事件

#### ABasePlayer
- 组件：UFallDamageComponent、UAutoRecoverComponent、UPhysicsControlComponent
- 弓相关：`bHasBow`（永久）、`bIsBowArmed`（模式）、`ABow* CurrentBow`
- `SetBowArmed(bool)` / `ToggleBowArmed()` - 切换弓箭模式

### 3. 抓取系统（Grab System）- Phase 3重构完成

#### 设计原则
- **职责分离**：Grabber 和 Grabbee 完全分离
- **接口驱动**：完全面向 IGrabbable 接口编程
- **PhysicsHandle 统一**：所有抓取类型都使用 UPhysicsHandleComponent（替代 PhysicsControl）
- **依赖注入**：PhysicsHandle 和 Inventory 由 BasePlayer 注入到 GrabHand
- **自动弓弦交互**：通过碰撞检测自动触发搭箭和抓弓弦

#### 抓取类型（EGrabType）

| GrabType | 实现方式 | 说明 |
|----------|---------|------|
| **None** | - | 不可抓取 |
| **Free** | PhysicsHandle | 物体保持抓取时的相对位置 |
| **WeaponSnap** | PhysicsHandle | 武器对齐到偏移位置（高强度跟随） |
| **HumanBody** | PhysicsHandle | 拖拽尸体，控制指定骨骼 |
| **Custom** | 自定义 | 通过 OnGrabbed/OnReleased 实现 |

**Phase 3 变更**：PhysicsControl → PhysicsHandle（更简单可靠）

#### Grabber 侧（UPlayerGrabHand）

```
USceneComponent
└── UPlayerGrabHand (抓取基类)
    ├── UPCGrabHand (PC模式 - 射线检测)
    └── UVRGrabHand (VR模式 - 球形检测、Gravity Gloves)
```

**关键属性**：
- `USphereComponent* HandCollision` - 手部碰撞体（tag: "player_hand"）
- `bool bIsRightHand` / `UPlayerGrabHand* OtherHand`
- `TMap<EWeaponType, FTransform> WeaponGrabOffsets`
- `AActor* HeldActor` / `EGrabType HeldGrabType`
- **依赖注入**：
  - `UPhysicsHandleComponent* CachedPhysicsHandle` - 由 BasePlayer 注入
  - `UInventoryComponent* CachedInventory` - 由 BasePlayer 注入
- PhysicsHandle参数：`FreeGrabStrength/Damping`、`WeaponSnapStrength/Damping`

**依赖注入方法**：
- `SetPhysicsHandle(UPhysicsHandleComponent*)` - 设置物理句柄
- `SetInventory(UInventoryComponent*)` - 设置库存组件

**核心接口（职责分离）**：
- `TryGrab(bool bFromBackpack)` - 决策层：找目标，决定用什么方式抓
- `TryRelease(bool bToBackpack)` - 决策层：决定如何释放
- `FindTarget(bool, FName&)` - 查找目标（子类实现）
- `GrabObject(AActor*, FName)` - 执行层：真正的抓取
- `ReleaseObject()` - 执行层：真正的释放

**内部实现（protected）**：
- `GrabFree()` - PhysicsHandle，保持相对变换
- `GrabWeaponSnap()` - PhysicsHandle（高强度），对齐到武器偏移
- `GrabHumanBody()` - PhysicsHandle，控制骨骼
- `ReleasePhysicsHandle()` - 释放 PhysicsHandle

#### UVRGrabHand（VR专用）

**重写方法**：
- `TryGrab()` - 自动背包检测 + Gravity Gloves 虚拟抓取决策
- `TryRelease()` - 虚拟抓取释放 + 箭自动存入背包
- `FindTarget()` - 背包/近距离/GravityGlovesTarget 优先级

**不重写**：`GrabObject()` - 执行层逻辑统一使用父类

**Gravity Gloves 功能**：
- 手里没东西时自动启用检测
- `GravityGlovesTarget` - 当前瞄准的远程目标（通过 Custom Stencil 高亮）
- `bIsVirtualGrabbing` - 虚拟抓取状态（Grip按下但物体未到手）
- `VirtualGrab(Target)` - 开始虚拟抓取，调用 OnGrabSelected
- `VirtualRelease(bLaunch)` - 结束虚拟抓取，bLaunch决定是否发射
- `CheckPullBackGesture()` - 检测后拉手势触发Launch
- **Launch机制**：向后拉动触发，调用 `AGrabbeeObject::LaunchTowards()`
- 参数：`GravityGlovesDistance`、`GravityGlovesAngle`、`PullBackThreshold`、`MinPullVelocity`、`LaunchArcParam`

**高亮实现**：
- `OnGrabSelected` / `OnGrabDeselected` - 设置 CustomDepthStencilValue (4/0)
- 后处理材质检测 Stencil Value 显示高亮效果

**背包检测**：
- `bIsInBackpackArea` - 通过 HandCollision overlap 检测（tag: "player_backpack"）

#### ABaseVRPlayer（VR玩家）

**手部旋转偏移**：
- `FRotator HandRotationOffset` - 蓝图可编辑的手部旋转偏移量
- `ApplyHandRotationOffset(bool bIsRight)` - 应用偏移（左右手镜像处理）
- 在 BeginPlay 中为两只手应用偏移

#### Grabbee 侧（IGrabbable 接口）

```
IGrabbable (接口)
├── AGrabbeeObject (可抓取物体基类)
│   └── AGrabbeeWeapon (武器基类)
│       ├── ABow (弓)
│       └── AArrow (箭)
└── ABaseEnemy (敌人基类)
```

**IGrabbable 接口方法**：
- `GetGrabType()` - 抓取类型
- `GetGrabPrimitive()` - 物理控制组件
- `CanBeGrabbedBy(Hand)` - 是否可抓取
- `SupportsDualHandGrab()` - 是否支持双手
- `OnGrabbed/OnReleased/OnGrabSelected/OnGrabDeselected` - 状态回调

**AGrabbeeObject 属性**：
- `EGrabType GrabType`、`bool bCanGrab`、`bool bSupportsDualHandGrab`
- `bool bIsHeld`、`UPlayerGrabHand* HoldingHand`
- `TSet<UPlayerGrabHand*> ControllingHands` - 双手抓取追踪

**AGrabbeeObject 方法**：
- `LaunchTowards(FVector Target, float ArcParam)` - 被重力手套发射，内部先清零速度再调用 OnGrabDeselected

**ABaseEnemy 实现**：
- `GetGrabType()` → HumanBody
- `CanBeGrabbedBy()` → bIsDead && !ControllingHands.Contains(Hand)
- `SupportsDualHandGrab()` → true
- `OnGrabbed/OnReleased` → 管理 ControllingHands

### 4. 弓箭系统（ABow & AArrow）

#### ABow（弓）

**组件**：StringMesh、StringCollision、BowFrontPosition、StringRestPosition、TrajectoryPreview

**状态**：
- `bool bBodyHeld / bStringHeld` - 弓身/弓弦抓取状态
- `UPlayerGrabHand* BodyHoldingHand / StringHoldingHand`
- `AArrow* NockedArrow` - 当前搭载的箭
- `float CurrentPullLength` - 当前拉弦长度

**核心函数**（Phase 2后）：
- `ReleaseString()` - 释放弓弦（发射）
- `NockArrow(AArrow*)` / `UnnockArrow()` - 搭箭/取消
- `SpawnAndNockArrow()` - PC模式生成箭
- `FireArrow()` - 发射箭
- `CalculateFiringSpeed()` - 计算发射速度
- `StartPullingString(Hand)` - **private**，由 OnGrabbed 内部调用
- `GetHandFromCollision(Comp)` - 辅助函数

**已移除**：`SetPullAmount()`、`UpdateStringFromHandPosition()`、`SetHandInStringArea()`

**IGrabbable 实现（动态）**：
- `GetGrabType()` → !bBodyHeld ? WeaponSnap : Custom
- `GetGrabPrimitive()` → !bBodyHeld ? MeshComponent : nullptr
- `SupportsDualHandGrab()` → true
- `CanBeGrabbedBy()` → 弓身检查 || 弓弦检查（!bStringHeld && Hand != BodyHoldingHand）

**OnStringCollisionBeginOverlap 自动逻辑**（Phase 2新增）：
1. 检测 "player_hand" tag
2. 获取手组件，验证不是 BodyHoldingHand
3. 检查 bBodyHeld && !bStringHeld
4. 如果手持箭：Hand->ReleaseObject() + NockArrow(Arrow)
5. 手抓弓弦：Hand->GrabObject(this)

#### AArrow（箭）

**状态（EArrowState）**：Idle、Nocked、Flying、Stuck


**核心函数**：
- `EnterIdleState/NockedState/FlyingState/StuckState`
- `CatchFire()` / `Extinguish()`

### 5. 玩家弓箭操作

#### PC 模式（ABasePCPlayer）

**状态**：`bIsAiming`、`bIsDrawingBow`

**流程**：
1. `SetBowArmed(true)` → 生成弓，LeftHand->GrabObject(Bow)
2. `StartAiming()` (LT按下) → 消耗箭，SpawnAndNockArrow()
3. `StartDrawBow()` (RT按下) → RightHand->GrabObject(Bow) → 触发 OnGrabbed → StartPullingString
4. Tick → Bow 自动跟随 StringHoldingHand 位置
5. `ReleaseBowString()` (RT松开) → RightHand->ReleaseObject() → OnReleased → ReleaseString → FireArrow

**已移除**：`StopDrawBow()`（已废弃）、手动 UpdateStringFromHandPosition 调用

#### VR 模式（ABaseVRPlayer）

**流程**：
1. 左手 Grip → 抓弓身
2. 右手拿箭，手移入弓弦区域 → OnStringCollisionBeginOverlap 自动：释放箭+搭箭+抓弓弦
3. 拉动手 → Bow::Tick 自动跟随
4. 松开 Grip → ReleaseObject → ReleaseString → FireArrow

**SetBowArmed 修改**：退出时使用 `ReleaseObject()` 而非已删除的 ReleaseAttachOnly

### 6. 武器类型（EWeaponType）
- None、Bow、Arrow

### 7. 箭的状态（EArrowState）
- Idle（闲置，可抓取）
- Nocked（搭在弓弦上）
- Flying（飞行中）
- Stuck（插在目标上）

## 当前进度

### 已完成 ✓
1. Effect 模块完全实现
2. Game 模块基础架构完成
3. 抓取系统核心逻辑完成
4. **Phase 2 抓取系统重构**（2025-12-31完成）：
   - 移除 Snap 类型，统一使用 PhysicsControl
   - WeaponSnap 改用 PhysicsControl（不再 Attach）
   - 添加 HandCollision 组件（用于弓弦 overlap 检测）
   - 移除 ReleaseAttachOnly、ValidateGrab、GrabSnap
   - BaseEnemy 添加 ControllingHands 追踪
   - Bow 移除 SetHandInStringArea/SetPullAmount/UpdateStringFromHandPosition
   - OnStringCollisionBeginOverlap 实现自动搭箭+抓弦
   - PC/VR 统一使用碰撞检测触发弓弦交互
5. **抓取系统代码清理**（2026-01-01完成）：
   - TryRelease 重构：VRGrabHand 简化为修改参数+单一父类调用
   - 职责分离：TryGrab/TryRelease（决策层）vs GrabObject/ReleaseObject（执行层）
   - VRGrabHand 移除 GrabObject 重写，VirtualGrab 决策移至 TryGrab
   - Gravity Gloves 简化：移除 bEnableGravityGloves（手空时自动启用）
   - 手部速度追踪优化：只在虚拟抓取状态才计算（非每帧）
6. **Phase 3 抓取系统重构**（2026-01-06完成）：
   - PhysicsControl → PhysicsHandle（更简单可靠）
   - 依赖注入模式：PhysicsHandle 和 Inventory 由 BasePlayer 注入到 GrabHand
   - 每只手预创建 PhysicsHandle（在 BasePlayer 中）
   - Gravity Gloves Launch 功能完善
   - 高亮实现：Custom Stencil + 后处理材质
   - 手部旋转偏移：`HandRotationOffset` 支持蓝图调参
7. **文件夹重构**（2026-01-06完成）：
   - `Grab/` → `Grabber/`（包含 IGrabbable.h）
   - GrabHand 相关文件从 `Game/` 移至 `Grabber/`
   - GrabTypes.h 移至 `Grabber/`
   - 所有 include 路径已更新

### 待实现
1. VR 弓弦交互测试验证
2. 特殊能力系统（鹰眼、定身术等）
3. 敌人AI系统
4. 场景互动与解谜

## 重要设计决策

1. **职责分离**：Grabber 和 Grabbee 彻底分离
2. **PhysicsHandle 统一**：所有抓取类型都用 PhysicsHandle，投掷手感自然
3. **依赖注入**：PhysicsHandle/Inventory 在 BasePlayer 创建，注入到 GrabHand
4. **接口驱动**：Grabber 侧完全面向 IGrabbable 接口
5. **自动弓弦交互**：通过 HandCollision overlap 自动触发，PC/VR 统一
6. **弓的双手抓取**：动态返回 GrabType（WeaponSnap/Custom），支持弓身+弓弦分开抓取

## 代码风格

- UE 命名规范（U前缀Component，A前缀Actor，E前缀Enum，F前缀Struct）
- 虚函数使用 `_Implementation` 后缀
- 蓝图可调用函数标记 `BlueprintCallable`
- 分段注释标记（`// ==================== 配置 ====================`）

## 注意事项

1. **蓝图导出文档可能不完全准确**：需要参考实际蓝图
2. **弓的特殊处理**：弓一旦获得就永久持有，不能真正丢弃
3. **PhysicsHandle 预创建**：每只手一个 PhysicsHandle，在 BasePlayer 的 BeginPlay 中创建并注入
4. **HandCollision 配置**：SphereRadius=5.0f，QueryOnly，tag="player_hand"
5. **蓝图参数生效时机**：手部旋转偏移等参数需在 BeginPlay 中应用，构造函数设置无效

## 协作模式

- **当前协作者**：Claude Code
- **协作方式**：阅读源码和设计文档，理解项目后协同开发
- **任务导向**：先完成核心系统，再打磨细节

---

**最后更新**：2026-01-06
**最近完成**：Phase 3 抓取系统重构（PhysicsHandle替代PhysicsControl、依赖注入、文件夹重构）
