# AI 系统重构文档

本文件夹包含敌人 AI 系统（GOAP + StateTree）的完整分析与重构方案。

## 文档导航

### 1. [当前架构分析](01_CurrentArchitecture_Analysis.md)
- AIController 的主要职责与问题
- Enemy Pawn 的主要职责与问题
- 架构痛点总结

### 2. [组件详细分析](02_Component_DetailAnalysis.md)
- 10 个组件的职责与设计评分
- 各组件问题汇总
- 组件改进建议

### 3. [MasterActor 架构问题](03_MasterActor_Analysis.md)
- 当前越级指挥问题
- 职责混乱分析
- 重构方案（事件广播 + 注册制）

### 4. [武器系统多态设计](04_Weapon_System_Design.md)
- 弓箭与近战的差异处理
- 统一攻击接口设计
- 分阶段攻击流程

### 5. [整体重构策略](05_Refactoring_Strategy.md)
- 分层重构方案
- Controller 精简
- Pawn 精简
- 组件职责收敛

### 6. [Subsystem 迁移方案](06_Subsystem_Migration.md)
- WorldSubsystem vs Actor 对比
- 迁移步骤
- 代码示例

---

## 重构优先级

| 优先级 | 阶段 | 主要任务 |
|--------|------|---------|
| P0 | Phase 1 | 攻击接口统一、组件通讯解耦 |
| P1 | Phase 2 | MasterActor → Subsystem、事件广播 |
| P2 | Phase 3 | 执行层优化、数据驱动配置 |

---

## 快速导航

- **问题最严重的模块**：Controller（职责过多）、MasterActor（越级指挥）
- **需要最紧急改进**：攻击系统（无统一接口）、MasterActor（每次全场搜索）
- **已经设计良好的**：HealthComponent、AISoundManager、EnemyUIComponent（事件驱动）

---

**生成日期**：2026年1月4日
