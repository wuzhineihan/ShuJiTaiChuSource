# MasterActor 架构分析

## 当前架构定位

`MasterActor` 是一个 **小队协调器 (Squad Coordinator)**，承担"指挥官"角色，负责协调多个敌人 AI 的群体行为。

---

## 核心职责

| 函数 | 职责 | 触发条件 |
|------|------|----------|
| `FoundEnemy` | 发现敌人（玩家） | 一个 Controller 感知到玩家 |
| `FoundSuspiciousLocation` | 发现可疑位置 | 听到声音、看到箭矢 |
| `LostEnemy` | 丢失目标 | 玩家脱离视线太久 |
| `RequestSupport` | 请求支援 | 号角手吹号角 |
| `NothingHappened` | 警报解除 | 警戒值归零 |

---

## 数据流分析

### 发现敌人流程 (FoundEnemy)

```
Controller 感知到玩家
    ↓
MasterActor.FoundEnemy(Controller, Location, EnemyRef, UIComponent)
    ↓
【越级指挥问题】直接操作 Controller 的内部变量：
    ├─ Set BIsCombat = true
    ├─ Set CurrentTargetEnemy = EnemyRef
    ├─ Set CurrentState = Fight
    └─ Set CombatWarningProcess = 100
    ↓
【越级指挥问题】直接操作 UI 组件：
    └─ UIComponent.EnterState()
    ↓
更新 GOAP 世界状态
    ├─ ChangeWorldState(EnemyLocation)
    └─ ChangeWorldState(ExecuteKillEnemyMission)
    ↓
触发重规划
    └─ StateTreeAI.SendStateTreeEvent(Goap.UpdatePlanning)
```

**核心问题**：MasterActor **越级直接操作** Controller 的内部状态和 UI 组件！

---

### 请求支援流程 (RequestSupport)

```
号角手 Controller 调用
    ↓
MasterActor.RequestSupport(Controller, Location, Radius)
    ↓
【性能问题】每次调用都搜索全场景：
    GetAllActorsOfClass(BP_GoapEnemy)
    ↓
筛选范围内的敌人
    ↓
【越级指挥】对每个敌人：
    ├─ Set CurrentState = SuperWarning
    ├─ UIComponent.EnterState()
    └─ 调用 FoundSuspiciousLocation()
```

**核心问题**：
1. 每次都搜索全场景，没有缓存
2. 直接修改其他 Controller 的状态

---

## 架构问题汇总

### 问题 A：越级指挥 (Bypassing Hierarchy)

```
正常层级:
MasterActor → Controller → Pawn → Component

当前实现:
MasterActor → Controller.变量 ❌
MasterActor → Component ❌
MasterActor 直接改 GOAP.WorldState ❌
```

**违反封装原则**：MasterActor 不应该知道 Controller 内部有 `BIsCombat`, `CurrentTargetEnemy`, `CombatWarningProcess` 这些变量。

---

### 问题 B：硬编码类型依赖

```cpp
// MasterActor 的参数类型
CallerControllerRef: Goap_EnemyAI_Controller_C  ❌ 具体类型
UIComponent: EnemyUIComponent_C                 ❌ 具体类型
EnemyArray: Array<BP_GoapEnemy_C>               ❌ 具体类型
```

**无法扩展**：如果新增不同类型的敌人（如 `BP_DogEnemy`, `BP_HorseEnemy`），MasterActor 无法处理。

---

### 问题 C：职责混乱

| MasterActor 当前做的事 | 应该由谁做 |
|------------------------|------------|
| 设置 `BIsCombat = true` | Controller 自己判断 |
| 设置 `CurrentTargetEnemy` | Controller 自己记录 |
| 调用 `UIComponent.EnterState()` | AlertnessComponent 广播事件 → UI 监听 |
| 更新 GOAP 世界状态 | Controller 自己更新 |
| 触发重规划 | Controller 自己触发 |

**MasterActor 实际上在"代替 Controller 做决策"**，而不是"协调 Controllers"。

---

### 问题 D：性能问题

```cpp
GetAllActorsOfClass(BP_GoapEnemy)  // 每次调用都搜全场景！
```

应该在 `BeginPlay` 时注册所有敌人到 `EnemyArray`，而不是每次都搜索。

---

### 问题 E：参数过多

```cpp
FoundEnemy(
    EnemyLocation, 
    EnemyRef, 
    CallerControllerRef,     // ← 为什么要传？
    UIComponent              // ← 为什么要传？
)
```

暴露了太多实现细节。应该只需要 `(Location, EnemyRef)`，其他组件自己查询。

---

## 重构方案

### 方案：轻量接口化 + 事件广播

**让 MasterActor 只发"指令"，不直接修改状态**

#### 1. 定义接口

```cpp
Interface: BPI_SquadMember
├── OnEnemySpotted(Location, EnemyRef) -> void
├── OnSuspiciousLocationReported(Location) -> void
├── OnAllyLostTarget(LastKnownLocation) -> void
└── OnSupportRequested(Location) -> void
```

#### 2. 重构 FoundEnemy

**当前**：
```blueprint
FoundEnemy(Controller, Location, EnemyRef, UIComponent):
    Set BIsCombat = true on Controller
    Set CurrentTargetEnemy = EnemyRef on Controller
    Set CurrentState = Fight on Controller
    UIComponent.EnterState()
    Controller.Goap.ChangeWorldState(...)
```

**重构后**：
```blueprint
FoundEnemy(Location, EnemyRef):
    // 只发消息，不修改内部变量
    Message(BPI_SquadMember).OnEnemySpotted(Location, EnemyRef)
    
    // 通知附近队友
    NearbyControllers = GetControllersInRadius(Location, 2000)
    For Each Controller in NearbyControllers:
        Message(BPI_SquadMember).OnEnemySpotted(Location, EnemyRef)
```

**Controller 内部处理**：
```blueprint
OnEnemySpotted(Location, EnemyRef):
    Set CurrentTargetEnemy = EnemyRef
    Set BIsCombat = true
    AlertnessComponent.EnterFightState()  // 广播事件给 UI
    Goap.UpdateWorldState()
    Goap.Replan()
```

---

#### 3. 改用注册制

**当前**：
```cpp
GetAllActorsOfClass(BP_GoapEnemy)  // 每次都搜全场景
```

**重构后**：
```cpp
// MasterActor.BeginPlay
RegisterAllSquadMembers():
    RegisteredControllers = GetAllActorsOfClass(AIController)
    For Each Controller in RegisteredControllers:
        If Implements(BPI_SquadMember):
            SquadMembers.Add(Controller)

// Controller.BeginPlay
MasterActor.RegisterMember(self)

// Controller.Destroyed
MasterActor.UnregisterMember(self)
```

---

#### 4. 重构后的函数签名

**之前**：
```cpp
FoundEnemy(EnemyLocation, EnemyRef, CallerControllerRef, UIComponent)
RequestSupport(CallerControllerRef, RequestLocation, RequestRadius)
LostEnemy(CallerControllerRef, EnemyLastKnownLocation)
```

**之后**：
```cpp
BroadcastEnemySpotted(Location, EnemyRef)
BroadcastSuspiciousLocation(Location)
BroadcastTargetLost(LastLocation)
BroadcastSupportRequested(Location, Radius)

GetControllersInRadius(Location, Radius) -> Array<AIController>
```

---

## Subsystem 替代方案（推荐）

不用 Actor，改用 **WorldSubsystem**，可以避免手动放置和管理。详见 `06_Subsystem_Migration.md`。

---

## 重构优先级

| 优先级 | 任务 |
|--------|------|
| P0 | 移除对 Controller 内部变量的直接访问 |
| P0 | 移除对 UIComponent 的直接调用 |
| P1 | 改用接口 `BPI_SquadMember` |
| P1 | 改用注册制，缓存 Controllers |
| P2 | 考虑迁移到 WorldSubsystem |
| P2 | 添加事件广播机制 |

---

## 总结评分

| 评估维度 | 评分 | 说明 |
|----------|------|------|
| **架构清晰度** | ⭐ | 越级指挥，职责不清 |
| **封装性** | ⭐ | 直接修改 Controller 内部变量 |
| **可扩展性** | ⭐ | 硬编码具体类型 |
| **性能** | ⭐⭐ | 每次调用都搜全场景 |
| **总评** | ❌ 需要彻底重构 |  |
