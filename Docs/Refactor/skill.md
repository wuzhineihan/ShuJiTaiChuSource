# 技能系统重构设计（Skill System Refactor）

> 目标：将旧蓝图技能系统重构为 C++ 方案，形成一套 **可复用（VR/PC 共享）**、**职责清晰**、**易扩展（策略模式）** 的技能框架。
>
> 范围说明：本文仅描述“技能系统框架”（学习/释放/绘制/互斥），暂不涉及任何具体技能（例如 Stasis/Freeze）的实现细节。

---

## 1. 总体原则与约束

### 1.1 不采用 GAS
本项目不引入 Gameplay Ability System（GAS）。技能系统以项目自研 C++ 框架实现，强调：
- 逻辑透明可控
- 与现有玩家/抓取系统低耦合
- 在 VR 与 PC 间尽可能复用

### 1.2 PC 的画符输入在 3D 世界中完成
PC 端符号绘制（Star Draw）采用 **3D 空间绘制**，输入源为 **玩家摄像机（CameraComponent）**。
- VR：输入源为某一只手（Hand / MotionController 相关的 `USceneComponent`）
- PC：输入源为相机（CameraComponent，同样继承 `USceneComponent`）

> 因此不需要为玩家额外抽象“输入源接口”，统一使用 `USceneComponent*` 作为输入源句柄即可。

### 1.3 技能绘制与抓取系统互斥
在交互层面保证以下互斥关系：
- **绘制中**：禁用抓取、释放、投掷等操作
- **抓取中/抓取交互发生时**：禁用绘制启动或强制停止绘制

目的：避免输入冲突、状态竞态、以及“绘制过程中误抓取/误释放”导致的不可控行为。

---

## 2. 核心模块与职责划分

技能系统框架由三部分组成：

1) **技能绘制管理器（StarDrawManager）**：只负责“星图绘制与识别”
2) **玩家技能组件（PlayerSkillComponent）**：负责“技能学习状态 + 技能释放路由 + 绘制状态维护”
3) **技能策略（Skill Strategy）**：用于承载“具体技能逻辑”，以策略模式扩展

### 2.1 `UPlayerSkillComponent`（玩家技能组件）
**挂载位置**：`ABasePlayer` 拥有 `UPlayerSkillComponent`（Composition）

**核心职责**：
- 维护玩家技能学习状态（原本在 PlayerState 的 LearnedMap）
- 作为技能释放入口与路由（根据技能枚举选择对应策略执行）
- 维护“当前是否在绘制”等运行时状态（例如 `bIsDrawing`、`bIsRightHandDrawing`）
- 统一处理绘制/抓取互斥（对外提供启停绘制时的 gating）

**关键状态（建议字段）**：
- `bool bIsDrawing;` 当前是否处于绘制中
- `bool bIsRightHandDrawing;` 当前是否用右手绘制（PC 也用此逻辑统一）
- `TObjectPtr<AStarDrawManager> StarDrawManager;` 当前绘制会话的管理器实例（运行时生成并保存）
- `TSet<ESkillType> LearnedSkills;` 或 `TMap<ESkillType, bool>`：技能学习状态（从 PlayerState 迁移至组件）

> 说明：学习状态存放位置从 PlayerState 迁移到 PlayerSkillComponent，是为了让技能系统更内聚，减少跨对象依赖。

### 2.2 `AStarDrawManager`（星图绘制管理器）
**类型选择**：保留为 `AActor`（与旧蓝图设计一致），由 `UPlayerSkillComponent` 在需要时 Spawn，并在绘制结束后销毁或复用。

**核心职责**（只做绘制与识别，不负责释放）：
- 接收绘制输入源（`USceneComponent* InputSource`）
- 驱动绘制的生命周期：Start / Tick Update / Finish
- 生成并管理绘制过程中的临时表现（例如 Niagara、临时点/线等）
- 在结束绘制时输出识别结果：返回一个“技能类型枚举”（例如 `ESkillType`）

**关键接口（建议）**：
- `void StartDraw(USceneComponent* InputSource);`
- `ESkillType FinishDraw();`（或返回 Optional/Invalid 值）

> 注意：StarDrawManager 不持有“玩家是否学会该技能”的判断逻辑，也不直接触发技能效果。

### 2.3 `USkillStrategyBase`（技能策略基类，策略模式）
**目的**：把“技能释放逻辑”从 PlayerSkillComponent 中分离出来，使扩展新技能时无需修改核心组件的控制流。

**核心职责**：
- 每个技能一个策略类，继承统一基类
- 通过统一入口执行技能：如 `Execute(ABasePlayer* Player, const FSkillContext& Context)`

**PlayerSkillComponent 的职责**：
- 维护 `SkillType -> Strategy` 的映射
- 在满足释放条件时，选择策略并调用 `Execute`

> 本阶段不实现具体技能，只建立接口与策略注册机制。

---

## 3. 类之间的关系（结构视图）

### 3.1 静态关系（Ownership / Reference）

- `ABasePlayer`
  - **拥有（has-a）** `UPlayerSkillComponent`

- `UPlayerSkillComponent`
  - **运行时创建并持有** `AStarDrawManager`（绘制会话期间有效）
  - **持有** `SkillType -> USkillStrategyBase` 的策略映射（或工厂/注册表）

- `AStarDrawManager`
  - **依赖** `USceneComponent* InputSource`（来自 Player：PC=Camera，VR=Hand）
  - **不依赖** PlayerState / LearnMap / 具体技能逻辑

### 3.2 交互关系（调用方向）

- Player（ABasePlayer）负责持有 `UPlayerSkillComponent`，并在输入绑定中调用 SkillComponent 的绘制/释放接口
- PlayerSkillComponent 负责状态维护与路由，并作为绘制（Start/Finish）与技能触发的统一入口
- StarDrawManager 负责识别并返回 SkillType
- SkillStrategy 接管具体技能逻辑执行

---

## 4. 玩家侧 API 设计（入口放置）

### 4.1 为什么 Start/Finish Draw 更适合放在 SkillComponent
我建议将“开始绘制/结束绘制”的对外入口放在 `UPlayerSkillComponent`，而不是 `ABasePlayer`，原因是：

- **高内聚**：绘制状态（`bIsDrawing`、`bIsRightHandDrawing`）、技能学习状态、技能释放路由都在 SkillComponent 内，入口放在组件内可以避免 Player 变成“状态转发器”。
- **跨平台复用更自然**：VR/PC 玩家类只需要提供 `InputSource`（Hand / Camera）并调用组件接口，不需要在每个 Player 子类里重复写 gating 与状态维护。
- **互斥逻辑更集中**：绘制与抓取互斥属于“技能系统的交互锁”，集中在 SkillComponent 更容易统一维护和测试。
- **降低 BasePlayer 的职责**：BasePlayer 作为通用玩家基类，尽量只提供能力与组件（composition），避免承载具体系统的控制流。

因此，建议的分工是：
- `ABasePlayer`：持有 `UPlayerSkillComponent`；负责输入绑定时调用组件接口；必要时提供查询抓取状态/战斗模式等信息给组件做 gating。
- `UPlayerSkillComponent`：提供 `StartStarDraw/FinishStarDraw` 为主入口，并管理完整绘制会话生命周期。

### 4.2 组件入口 API
推荐由 `UPlayerSkillComponent` 提供两类入口：

- `UPlayerSkillComponent::StartStarDraw(USceneComponent* InputSource)`
- `UPlayerSkillComponent::FinishStarDraw()`

> InputSource 的传入方式不变：PC 传 CameraComponent，VR 传对应的 Hand 组件。

---

## 5. PC/VR 的手选择与限制策略

### 5.1 `bIsRightHandDrawing` 统一表示“绘制手”
无论 PC/VR，都使用 `UPlayerSkillComponent::bIsRightHandDrawing` 表达当前绘制来源。

- PC 默认使用右手绘制
- 若右手不空闲（例如右手正在持有物体），则切换为左手绘制

> “手是否空闲”的具体判定依赖抓取系统状态（例如 GrabHand 是否 holding）。

### 5.2 PC 弓箭模式限制
PC 在弓箭模式（BowArmed）下 **禁止启动绘制**。
- 仅在徒手模式（Unarmed）允许开始绘制

该门禁逻辑建议放在 `ABasePCPlayer::StartStarDraw` 入口或 `UPlayerSkillComponent` 的 gating 逻辑中（两者任选其一，但建议最终统一由 SkillComponent 做判定，保持调用侧简单）。

---

## 6. 关键流程（时序）

### 6.1 开始绘制流程
1. 玩家输入触发后，调用 `PlayerSkillComponent->StartStarDraw(InputSource)`
2. `UPlayerSkillComponent` 进行门禁检查：
   - 是否已经在绘制（`bIsDrawing`）
   - 是否允许绘制（例如 PC 非弓箭模式）
   - 是否允许与抓取互斥（当前是否能禁用抓取）
3. `UPlayerSkillComponent` 更新状态：
   - 设置 `bIsDrawing = true`
   - 选择绘制手并写入 `bIsRightHandDrawing`
4. `UPlayerSkillComponent` Spawn 并保存 `AStarDrawManager`
5. 调用 `StarDrawManager->StartDraw(InputSource)` 进入绘制会话
6. 绘制期间：禁用抓取/释放/投掷等交互

### 6.2 结束绘制流程
1. 玩家输入触发后，调用 `PlayerSkillComponent->FinishStarDraw()`
2. `UPlayerSkillComponent` 更新状态（先确保从“绘制态”退出）：
   - `bIsDrawing = false`
3. 调用 `ESkillType Skill = StarDrawManager->FinishDraw()` 获取识别结果
4. 销毁或回收 `StarDrawManager`
5. 若 `Skill` 有效：
   - 进入 `UPlayerSkillComponent` 的“技能触发逻辑”
   - 检查学习状态（是否已学会）
   - 找到对应策略并执行：`Strategy->Execute(...)`
6. 恢复抓取等交互能力

---

## 7. 绘制与抓取互斥（建议实现要点）

### 7.1 绘制期间禁用抓取
建议由 `UPlayerSkillComponent` 统一对外发布“交互锁”状态，例如：
- `bool IsDrawing() const;`

抓取系统在执行 Grab/Release/TryThrow 前检查该状态：
- 若 `IsDrawing == true`：直接拒绝（return）

### 7.2 抓取触发时禁用绘制
当抓取系统发生以下事件之一：
- 开始抓取
- 正在持有物体

则 SkillComponent：
- 禁止启动绘制


