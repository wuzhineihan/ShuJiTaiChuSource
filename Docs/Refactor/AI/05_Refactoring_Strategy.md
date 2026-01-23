# 整体重构策略

## 分层重构目标

重构的核心是**"切割职责、消除耦合、事件驱动"**，使系统从"上帝类+强耦合"变成"组件化+弱耦合"。

```
重构前：
Controller（大脑） ←→ Pawn（躯体）  双向硬编码依赖
    ↓                      ↓
处理感知            直接改状态、UI

重构后：
Controller（决策器）  →  组件（执行者）  通过接口单向通讯
    ↓                         ↓
接收消息、触发 GOAP       广播事件，外部监听
```

---

## Controller 精简方案

### 当前职责（冗余）
- ✅ 感知处理 (感知事件 → 分类)
- ❌ 警戒值计算 (应由 AlertnessComponent)
- ❌ UI 更新触发 (应由事件驱动)
- ❌ 旋转控制 (应由 RotationComponent)
- ❌ 音效触发 (应由事件驱动)
- ❌ 武器初始化 (应由 Pawn/WeaponComponent)
- ✅ GOAP 规划 & StateTree 调度

### 重构后职责（精简）

| 职责 | 实现方式 |
|------|----------|
| 感知入口 | 保留 `OnTargetPerceptionUpdated` |
| 感知分类 | 新建 `PerceptionHandlerComponent` 处理 |
| 获取感知结果 | 通过组件接口查询（如 `GetTargetInformation`） |
| 决策与规划 | 保留 GOAP 规划与重规划触发 |
| 行为调度 | 保留 StateTree/TaskExecutor 调度 |
| 通讯 | 通过接口 `IEnemyAIController` 接收指令 |
| 事件监听 | 监听 AlertnessComponent、MasterActor 的事件 |

### 代码示例

```blueprint
// 新的 Goap_EnemyAI_Controller

BeginPlay():
    Enemy = GetPawn()
    AlertnessComp = GetComponentByClass(AlertnessComponent)
    PerceptionHandler = NewObject(PerceptionHandlerComponent)
    
    // 绑定事件监听
    Bind AlertnessComp.OnStateChanged -> Self.HandleStateChanged
    Bind MasterActor.OnEnemySpottedBySquad -> Self.HandleEnemySpotted

// 感知处理（精简）
OnTargetPerceptionUpdated(Actor, Stimulus):
    SenseType = GetSenseClass(Stimulus)
    
    // 转发给 PerceptionHandler
    PerceptionHandler.HandlePerception(SenseType, Stimulus, Actor)

// 事件响应
HandleStateChanged(NewState):
    Switch NewState:
        Fight:
            Goap.UpdateWorldState(InCombat=True)
            Goap.Replan()
        Warning:
            Goap.UpdateWorldState(Alert=True)
        Idle:
            Goap.UpdateWorldState(InCombat=False, Alert=False)

HandleEnemySpotted(Location, EnemyRef):
    // 不再直接改状态！
    // AlertnessComponent 会自动处理状态转换
    // 只需通知感知系统
    PerceptionHandler.ReceiveSquadReport(Location, EnemyRef)
```

---

## Pawn 精简方案

### 当前状态变量（过多）

```cpp
BEnemyDead, BInStasis, BIsAttacking, BHeavyArmor, BCanBugle
HasTorch, SleepEnemy, sleepnear, Patrol
// ...共 30+ 个变量
```

### 重构后状态归属

| 状态 | 新位置 | 理由 |
|------|--------|------|
| `BEnemyDead` | HealthComponent | 生命值为 0 即为死亡 |
| `BInStasis` | StatusComponent | 时停是一种临时状态 |
| `BIsAttacking` | CombatComponent 或 Pawn（只保留一份） | 不需要同步两份 |
| `SleepEnemy` | StatusComponent | 睡眠是临时状态 |
| `HasTorch` | Pawn（配置） | 初始属性 |
| `BCanBugle` | Pawn（配置） | 初始属性 |
| `Patrol` | Pawn（配置） | 初始属性 |

### Pawn 职责（精简）

| 职责 | 实现 |
|------|------|
| 为组件容器 | 持有 Health、Damage、Alertness、Combat 等组件 |
| 暴露组件接口 | 让 Controller 通过接口访问组件功能 |
| 坐标转发 | 不需要复杂逻辑，大部分由组件实现 |
| 受伤响应 | 接收 `TakeDamage` 接口调用 → 转发给 DamageComponent |
| 死亡响应 | 监听 HealthComponent 的 `OnDeath` 事件 |
| 初始化 | 初始化组件、加载配置 |

### 代码示例

```blueprint
// 新的 BP_EnemyBase (可作为基类)

Components:
    HealthComponent
    DamageComponent
    AlertnessComponent
    CombatComponent
    StatusComponent
    RotationComponent
    ... 其他功能组件

BeginPlay():
    // 初始化配置（从 DataAsset 加载）
    LoadEnemyConfig()
    
    // 绑定组件事件
    HealthComponent.OnDeath += HandleDeath
    DamageComponent.OnDamageApplied += HandleDamageApplied
    StatusComponent.OnStatusChanged += HandleStatusChanged

// 简单的接口实现
TakeDamage(DamageInfo):
    DamageComponent.ApplyDamage(DamageInfo)

IsAlive() -> bool:
    return HealthComponent.IsAlive()

// 由组件自己处理的逻辑，Pawn 不干预
// - 攻击执行 (CombatComponent)
// - 旋转 (RotationComponent)
// - 睡眠/时停 (StatusComponent)
```

---

## 组件职责收敛

### AlertnessComponent（重构）

**输入**：
- `RequestIncreaseAlertness(Intensity)` 
- `RequestDecreaseAlertness()`
- `SetEnemyState(NewState)`

**输出事件**：
- `OnWarningProgressChanged(NewProgress)`
- `OnEnemyStateChanged(OldState, NewState)`

**职责边界**：
- ✅ 警戒值数值计算
- ✅ 状态机管理 (Idle → Warning → Fight)
- ❌ 感知分类 (移到 PerceptionHandler)
- ❌ UI 更新 (由 EnemyUIComponent 监听事件)

### 新建 PerceptionHandlerComponent

**职责**：
- 感知事件分类 (视觉/听觉/尸体)
- 信号强度判断
- 调用 `AlertnessComponent.RequestIncreaseAlertness()`

**优点**：
- 使 AlertnessComponent 逻辑简单
- 感知处理独立可测试

### 新建 CombatComponent

**职责**：
- 攻击选择 (通过 `BPI_Attackable` 接口)
- 目标管理
- 距离判定 (远程/近战/超近)
- 聚焦管理

### 新建 StatusComponent

**职责**：
- 睡眠/时停/眩晕等临时状态
- 状态相关的逻辑（如睡眠时禁用感知）

### 新建 RotationComponent

**职责**：
- 战斗旋转 vs 移动旋转切换
- 面向目标的平滑转身

---

## 数据与配置驱动

### DataAsset：EnemyConfig

```
DA_EnemyConfig_Archer:
  Components:
    - ArcherComponent
    - MeleeComponent (可选)
  Health: 100
  Speed: 400
  DefaultWeapon: Bow
  CanBugle: False
  SoundDataTable: DT_ArcherSounds
  
DA_EnemyConfig_HornBlower:
  Components:
    - MeleeComponent
    - BugleComponent
  Health: 80
  Speed: 350
  DefaultWeapon: Sword
  CanBugle: True
  SoundDataTable: DT_HornSounds
```

### Pawn 初始化

```blueprint
Initial(Config: EnemyConfig):
    // 根据配置动态添加组件
    For Each ComponentClass in Config.Components:
        AddComponent(ComponentClass)
    
    // 配置参数
    Health = Config.Health
    MaxSpeed = Config.Speed
    CanBugle = Config.CanBugle
    ...
```

---

## 事件驱动通讯

### 弃用：直接调用

```cpp
// ❌ 旧方式
Controller.PlaySFX()
Controller.SetAttackState(true)
Controller.BeDamaged()
UIComponent.UpdateAlertBar()
```

### 采用：事件广播

```cpp
// ✅ 新方式
AlertnessComponent.OnStateChanged.Broadcast(NewState)
    → PerceptionHandler 监听 (调整感知参数)
    → AISoundManager 监听 (播放对应音效)
    → EnemyUIComponent 监听 (更新 UI)

DamageComponent.OnDamageApplied.Broadcast(DamageInfo)
    → RotationComponent 监听 (播放受击反应)
    → UIComponent 监听 (更新血条)

HealthComponent.OnDeath.Broadcast()
    → Pawn 监听 (销毁组件)
    → Controller 监听 (停止 AI)
```

---

## 接口统一

### IEnemyController

```cpp
Interface IEnemyController
{
    // 来自 MasterActor
    UFUNCTION(BlueprintNativeEvent)
    void OnEnemySpottedBySquad(FVector Location, AActor* Enemy);
    
    UFUNCTION(BlueprintNativeEvent)
    void OnSuspiciousLocationReported(FVector Location);
    
    UFUNCTION(BlueprintNativeEvent)
    void OnAllyRequestSupport(FVector Location);
}
```

### IEnemyPawn

```cpp
Interface IEnemyPawn
{
    UFUNCTION(BlueprintNativeEvent)
    void TakeDamage(const FDamageInfo& DamageInfo);
    
    UFUNCTION(BlueprintNativeEvent)
    bool IsAlive();
    
    UFUNCTION(BlueprintNativeEvent)
    void SetAttackState(bool bIsAttacking);
}
```

---

## 重构分阶段

### Phase 1（基础）：攻击与感知解耦
- [ ] 定义 `BPI_Attackable` 接口
- [ ] ArcherComponent、MeleeComponent 实现接口
- [ ] 移除 Pawn 的 `Switch on EnemyType` 硬编码
- [ ] 新建 `PerceptionHandlerComponent`
- [ ] 从 Controller 分离感知分类逻辑

### Phase 2（核心）：事件驱动与状态集中
- [ ] AlertnessComponent 添加委托事件
- [ ] EnemyUIComponent 完全监听事件（移除外部调用）
- [ ] HealthComponent 添加 `OnHealthChanged/OnDeath` 事件
- [ ] 新建 `CombatComponent`、`StatusComponent`
- [ ] Controller 改为监听事件而非直接操作

### Phase 3（协调）：MasterActor 重构
- [ ] 改为 WorldSubsystem（或保持 Actor 但改接口）
- [ ] 移除直接改 Controller 状态的代码
- [ ] 改用接口 `BPI_SquadMember` + 事件广播
- [ ] 实现注册制替代全场景搜索

### Phase 4（优化）：数据驱动与执行层
- [ ] 创建 `EnemyConfig` DataAsset
- [ ] Pawn 根据配置动态添加组件
- [ ] 评估是否用自定义 TaskExecutor 替代 StateTree

---

## 预期收益

| 指标 | 当前 | 重构后 |
|------|------|--------|
| **Controller 行数** | 800+ | 300-400 |
| **Pawn 行数** | 760+ | 300-400 |
| **组件耦合度** | 高 | 低 |
| **新增敌人时间** | 改多处代码 | 新建 DataAsset + 选择组件 |
| **新增攻击类型** | 改 Pawn + 组件 | 只需新建组件 + 实现接口 |
| **可测试性** | 困难 | 容易 |

---

## 风险与缓解

| 风险 | 缓解方案 |
|------|----------|
| 重构期间 AI 功能破裂 | 分阶段、保留旧代码直到新的完全通过 |
| 性能下降（事件广播） | 使用事件池、避免过度广播 |
| 调试难度增加 | 详细的日志、可视化调试工具 |

