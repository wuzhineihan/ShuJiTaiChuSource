# 组件详细分析

共发现 **10 个组件**，分为以下几类：

| 类别 | 组件 | 位置 |
|------|------|------|
| **生命与伤害** | HealthComponent, DamageComponent | `_Component/` |
| **感知与警戒** | AlertnessComponent | `_Component/` |
| **UI** | EnemyUIComponent | `_Component/` |
| **动画** | AnimationComponent | `_Component/` |
| **攻击** | ArcherComponent, MeleeComponent | `Goap_AI/WeaponManagerComponent/` |
| **音效** | AISoundManager | `Goap_AI/AISound/` |
| **移动** | AnimalMovementComponent, SteeringBehaviourComponent | `_Component/` |

---

## 1. HealthComponent ✅ 优秀

| 项目 | 内容 |
|------|------|
| **职责** | 管理生命值 |
| **变量** | `Max_health`, `current_health` |
| **函数** | `damage()`, `SetHealth()`, `GetHealth()` |
| **代码行数** | 简洁 |
| **评价** | ✅ 职责单一、无依赖、易复用 |

**缺点**：
- 无事件广播（死亡时不通知外部）
- 调用者需要检查返回值来判断死亡

**改进建议**：
```cpp
// 添加委托
OnHealthChanged.Broadcast(CurrentHealth, MaxHealth)
OnDeath.Broadcast()
```

---

## 2. DamageComponent ✅ 良好

| 项目 | 内容 |
|------|------|
| **职责** | 计算伤害倍率、应用伤害 |
| **依赖** | `HealthComponent`, `BPI_CommonInterface` |
| **函数** | `SetDamageRate()`, `ApplyDamage()` |
| **评价** | 职责清晰，但有轻微问题 |

**问题**：
- 通过接口消息查询 `AreYouAlive`，增加了通讯开销
- 应该直接从 `HealthComponent.IsAlive()` 获取

**改进建议**：
```cpp
// 改为
If (HealthComponent.IsAlive()):
    ApplyDamage()
// 而不是通过接口消息
```

---

## 3. AlertnessComponent ⚠️ 功能完整但过复杂

| 项目 | 内容 |
|------|------|
| **职责** | 管理警戒值、感知处理、状态切换 |
| **变量** | 21 个（过多） |
| **委托** | ✅ `WarningProgressPublisher`, `EnemyStateUpdate` |
| **代码行数** | 673 行（过长） |
| **评价** | ⚠️ 功能完整但职责混杂 |

**问题**：
1. **感知分类逻辑混在一起**：`PlayerSenseAlertness`, `BodySenseAlertness`, `HearingSenseAlertness` 都在这个组件内
   - 感知的分类应该由外部（Controller 或 PerceptionHandler）处理
   - 只需传入"感知类型 + 强度"

2. **硬编码 Pawn 类型**：
   ```cpp
   Cast<BP_GoapEnemy>()  // ❌ 无法复用
   ```

3. **变量命名不一致**：`Phase1/2/3/4IncreaseMultiplier` 命名晦涩

**改进建议**：
- 感知分类逻辑 → 新建 `PerceptionHandlerComponent`
- AlertnessComponent 只负责数值计算 + 状态机
- 输入：`RequestAlertIncrease(Intensity)`, `RequestAlertDecrease()`
- 输出：`OnStateChanged`, `OnWarningProgress` 委托

---

## 4. EnemyUIComponent ✅ 设计良好

| 项目 | 内容 |
|------|------|
| **职责** | 管理敌人 UI（血条、警戒条） |
| **亮点** | ✅ 监听 `AlertnessComponent` 的委托来更新 UI |
| **代码** | `Bind Delegate(On_EnemyStateUpdate) to EnemyStateUpdate on OwnerAlertnessComponent` |
| **评价** | ✅ 这是正确的事件驱动设计 |

**问题**：
- `UpdateAlertBar()` 仍被 Controller 直接调用
- 如果 AlertnessComponent 已广播事件，不应该再让外部调用

**改进建议**：
- 移除 `UpdateAlertBar()` 的外部调用
- 完全依赖 AlertnessComponent 的委托

---

## 5. AnimationComponent ⚠️ 职责混乱

| 项目 | 内容 |
|------|------|
| **名义职责** | 动画管理 |
| **实际职责** | 播放闲置动画、Sequencer 播放、生成瓶子道具 |
| **变量** | `sequencer_player`, `HasBottle`, `VaseRef` |
| **评价** | ❌ 职责混乱、命名误导 |

**问题**：
1. **生成瓶子道具不属于"动画"**
   - 应该在 Pawn 或专门的 `PropComponent` 处理

2. **名字叫 AnimationComponent 但只处理闲置/表演**
   - 攻击、受伤、移动动画都在其他地方

3. **依赖 GlobalGameInstance（未使用）**

**改进建议**：
- 重命名为 `IdleAnimationComponent` 或 `PerformanceComponent`
- 如有道具生成，创建 `PropComponent` 或交由 Pawn 处理
- 清理未使用的依赖

---

## 6. ArcherComponent ⚠️ 功能完整但缺接口

| 项目 | 内容 |
|------|------|
| **职责** | 弓箭攻击全套：装备、瞄准、射击、装填 |
| **函数** | `CreateEnemyWeapon()`, `Equipe_Archer()`, `Archer_Reload()`, `Archer_Attack()` |
| **Tick** | 处理拉弦动画、箭矢位置同步 |
| **评价** | ⚠️ 功能完整但没有实现统一接口 |

**问题**：
1. **没有实现统一的攻击接口**
   - 无法被"攻击选择器"统一调用

2. **在 Tick 中持续更新箭矢位置**
   - 性能开销（每帧 Update 变换）

3. **攻击状态复杂**
   - 拉弦 → 瞄准 → 射击，多阶段但没有清晰的接口暴露

**改进建议**：
- 实现 `BPI_Attackable` 接口
  ```cpp
  GetAttackRange() -> 1500
  CanAttack(Target) -> HasAmmo && !OnCooldown
  PrepareAttack(Target) -> AttackHandle
  CommitAttack(Handle) -> float (返回动画时间)
  GetPriority() -> 20
  ```

---

## 7. MeleeComponent ❌ 功能残缺

| 项目 | 内容 |
|------|------|
| **职责** | 近战武器管理 |
| **函数** | `CreateWeapon()`, `OnEnemyDead()` |
| **评价** | ❌ 功能残缺、缺少攻击函数 |

**问题**：
1. **没有攻击函数！**
   - 近战攻击逻辑在 `BP_GoapEnemy.Attack` 接口里播放蒙太奇
   - 两套系统，容易混淆

2. **只负责生成武器和死亡时掉落**
   - 没有攻击触发、伤害判定、CD 管理

**改进建议**：
- 添加 `MeleeAttack(Target) -> float` 函数
- 将 `BP_GoapEnemy.Attack` 里的近战逻辑迁移过来
- 实现 `BPI_Attackable` 接口

---

## 8. AISoundManager ✅ 优秀

| 项目 | 内容 |
|------|------|
| **职责** | 管理 AI 音效播放 |
| **函数** | `PlaySoundByLibrary()`, `ChoseSound()`, `SetDataTable()` |
| **数据驱动** | ✅ 使用 DataTable 配置音效 |
| **评价** | ✅ 设计良好、数据驱动、易扩展 |

**优点**：
- 通过 DataTable 灵活配置
- 概率选择（MustToPlay 参数控制是否一定播放）
- 通过接口获取 AudioComponent

**无需改进** ✅

---

## 9. AnimalMovementComponent ✅ 专用且清晰

| 项目 | 内容 |
|------|------|
| **职责** | 动物（马）的移动控制 |
| **函数** | `AddHorseInput()`, `UpdateMovementInput()`, `SetMaxSpeed()` |
| **Tick** | 平滑插值速度和转向 |
| **评价** | ✅ 适用于坐骑类 AI，职责清晰 |

**优点**：
- 职责单一（只管动物移动）
- 参数化设计（速度/转向加速度可配置）
- 使用曲线编辑器（YawRotationRateCurve）

**无需改进** ✅

---

## 10. SteeringBehaviourComponent ✅ 专用且清晰

| 项目 | 内容 |
|------|------|
| **职责** | 转向行为（躲避/追逐向量计算） |
| **函数** | `AddChaseVector()`, `AddAwayVector()`, `CalculateMoveVector()` |
| **评价** | ✅ 用于群体 AI，职责清晰 |

**优点**：
- 只负责向量计算
- 可用于群体行为
- 易于组合多种行为

**无需改进** ✅

---

## 组件评分表

| 组件 | 职责清晰 | 解耦程度 | 可复用性 | 总评 |
|------|----------|----------|----------|------|
| HealthComponent | ⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐ | ✅ 优秀 |
| DamageComponent | ⭐⭐⭐ | ⭐⭐ | ⭐⭐⭐ | ✅ 良好 |
| AISoundManager | ⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐ | ✅ 优秀 |
| EnemyUIComponent | ⭐⭐⭐ | ⭐⭐ | ⭐⭐ | ✅ 良好 |
| AlertnessComponent | ⭐⭐ | ⭐ | ⭐ | ⚠️ 需重构 |
| ArcherComponent | ⭐⭐ | ⭐⭐ | ⭐ | ⚠️ 需接口化 |
| MeleeComponent | ⭐ | ⭐⭐ | ⭐ | ❌ 功能残缺 |
| AnimationComponent | ⭐ | ⭐ | ⭐ | ❌ 职责混乱 |
| AnimalMovementComponent | ⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐ | ✅ 优秀 |
| SteeringBehaviourComponent | ⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐ | ✅ 优秀 |

---

## 组件改进优先级

| 优先级 | 组件 | 任务 |
|--------|------|------|
| P0 | ArcherComponent, MeleeComponent | 实现统一攻击接口 |
| P0 | AlertnessComponent | 拆分感知处理逻辑 |
| P1 | AnimationComponent | 拆分/重命名，移除道具生成 |
| P1 | HealthComponent | 添加 OnHealthChanged/OnDeath 委托 |
| P2 | DamageComponent | 移除接口查询，直接使用 HealthComponent |
| - | AISoundManager, AnimalMovementComponent, SteeringBehaviourComponent | 保持现状 ✅ |
