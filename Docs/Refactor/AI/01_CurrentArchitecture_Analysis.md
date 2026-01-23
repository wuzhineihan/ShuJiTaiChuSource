# 当前架构分析

## AIController 职责总结

### 1. 持有的组件

| 组件 | 职责 |
|------|------|
| `StateTreeAI` | 状态树执行 |
| `Goap` | GOAP 规划器 |
| `AIPerception` | 感知系统 |

### 2. 承担的主要任务

| 任务类别 | 具体职责 | 涉及函数/事件 |
|----------|----------|--------------|
| **初始化** | 获取 Pawn、UI、音效、MasterActor 引用 | `BeginPlay` |
| **感知处理** | 处理视觉/听觉/尸体感知，区分玩家和其他敌人 | `OnTargetPerceptionUpdated` + 5个 Macro (`PlayerSense`, `HearSense`, `BodySense`, `CombatSense`, `DebugSightSense`) |
| **警戒值管理** | 增减警戒条，触发状态切换 | `UpdateAlertbar`（通过接口调用） |
| **状态转换** | 根据警戒值通知 `MasterActor`，更新 `EnemyState` | Macro 内部调用 `MasterActor.FoundEnemy` / `FoundSuspiciousLocation` |
| **GOAP 规划** | 调用 `Goap.FindGoal()` 和 `Goap.Call_Planner()` | `Goap_Planning` |
| **GOAP 世界状态维护** | 更新距离、敌人存活、丢失目标等布尔值 | `CheckTheDistanceOfTheEnemy`, `CheckEnemyLostOrNot`, `CheckEnemyIsAlive` |
| **旋转控制** | 战斗时面向敌人，非战斗时跟随移动方向 | `Rotate_Setting`, `SetEnemyRotation` (在 Tick 中调用) |
| **武器类型初始化** | 根据 `EnemyType` 创建武器并设置 GOAP 状态 | `SetEnemyType` |
| **音效播放** | 代理调用 `AISoundManager` | `PlaySFX` |
| **受伤响应** | 被攻击时通知 `MasterActor` | `BeDamaged` |
| **死亡处理** | 销毁 Controller | `EnemyDeath` |

### 3. 硬编码的类型依赖

```cpp
Cast<BP_GoapEnemy>()           // 硬编码 Pawn 类型
GetComponentByClass<HealthComponent>()  // 硬编码组件
→ 无法复用于其他敌人类型
```

---

## BP_GoapEnemy (Pawn) 职责总结

### 1. 承担的主要任务

| 任务类别 | 具体职责 | 涉及函数/事件 |
|----------|----------|--------------|
| **初始化** | 合并骨骼网格、创建武器、获取 Controller 引用、初始化 UI 血条、设置音效表 | `Initial`, `SetSoundDataTable`, `CheckNormalAction` |
| **受伤处理** | 播放受击动画、唤醒睡眠、应用伤害、更新 UI、通知 Controller、判断死亡 | `BeDamaged`, `DamageHitReact`, `TakeDamage`(接口) |
| **死亡处理** | 停止动画、销毁组件、开启布娃娃、通知 Controller | `Enemy Die` |
| **攻击执行** | 根据 `EnemyType` 分发到 `ArcherComponent` 或播放近战动画 | `Attack`(接口) |
| **睡眠系统** | 进入/退出睡眠状态、检测玩家靠近 | `Sleep`, `StopSleep`, `SleepBox` 重叠事件 |
| **火把系统** | 生成并附着火把 | `AttachTorch` |
| **时停系统** | 进入/退出时停状态 | `EnterStasis`, `ExitStasis`(接口) |
| **状态同步** | 将攻击状态同步给 Controller | `SetAttackState` |
| **Tick 优化** | 根据是否可见调整 Tick 间隔 | `Tick` |

### 2. 状态变量过多（30+个）

```cpp
BEnemyDead, BInStasis, BIsAttacking, BHeavyArmor, BCanBugle
HasTorch, SleepEnemy, sleepnear, Patrol
EnemyType, EnemyMaxHealth
ChaseSpeed, PatrolSpeed, NormalSpeed
// ... 还有多个同步到 Controller 的变量
```

---

## 核心架构问题

### 问题 1：双向强耦合

```
Enemy <--> Controller
```

**Enemy 调用 Controller**：
```cpp
Goap_EnemyAI_Controller.PlaySFX()
Goap_EnemyAI_Controller.BeDamaged()
Goap_EnemyAI_Controller.EnemyDeath()
设置 BIsAttacking 到 Controller
```

**Controller 调用 Enemy**：
```cpp
Enemy Character.ArcherComponent.Archer_Attack()
Enemy Character.AnimationComponent.StopPlaying()
Enemy Character.Can_Hear = false
```

**问题**：无法独立测试任何一方，改一个地方可能导致连锁故障。

---

### 问题 2：上帝类（God Class）

**Controller**：感知 + 警戒 + UI 触发 + 音效 + 旋转 + 武器初始化 + GOAP 规划

**Pawn**：伤害系统 + 睡眠 + 火把 + 时停 + 攻击分发 + 死亡 + UI 初始化

每个类做的事情都太多，职责边界模糊。

---

### 问题 3：感知处理复杂度高

Controller 的 `OnTargetPerceptionUpdated` 包含 5 个 Macro（PlayerSense, HearSense, BodySense, CombatSense, DebugSightSense），嵌套逻辑深，难以维护。

---

### 问题 4：攻击逻辑硬编码

```cpp
Switch on Enum Enemy_Type (EnemyType)
    |-- Archer_Enemy: ArcherComponent.Archer_Attack()
    |-- Normal_Enemy: PlayAnimMontage(Attack_PrimaryA_Montage)
```

每增加一种敌人或攻击方式，都要修改这个 Switch。

---

### 问题 5：状态变量散落且重复

| 变量 | 存储位置 | 问题 |
|------|---------|------|
| `BIsAttacking` | Enemy + Controller | 需要手动同步 |
| `CurrentState` | Controller | Controller 直接改，UI 没有通知 |
| `BIsCombat` | Controller | MasterActor 直接改 |

---

### 问题 6：GOAP 状态散落各处

`Goap.ChangeWorldState()` 被分散调用在多个地方：
- `init_patrol_state`
- `SetEnemyType`
- `CheckTheDistanceOfTheEnemy`
- `CheckEnemyLostOrNot`

难以追踪"当前世界状态什么时候改变"。

---

## 小结

| 问题 | 影响 | 严重度 |
|------|------|--------|
| 双向耦合 | 无法独立测试/复用 | 🔴 严重 |
| 上帝类 | 代码复杂度高、难维护 | 🔴 严重 |
| 硬编码类型 | 无法扩展新敌人 | 🟠 中等 |
| 感知处理复杂 | 容易出 Bug、难调试 | 🟠 中等 |
| 攻击不统一 | 新增攻击方式需改多处 | 🟠 中等 |
| 状态重复存储 | 数据同步易出错 | 🟡 轻微 |
