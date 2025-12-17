# 开发路线图：PC 模式支持与架构重构

本路线图旨在指导将核心 Gameplay 逻辑与 VR 具体实现解耦，从而实现代码复用以支持 PC 桌面模式，并提高项目的长期可维护性。

## 第一阶段：解耦 Gameplay 与 VR（重构基础）

目标：建立通用的角色基类和组件系统，将非 VR 特有的逻辑从 `BP_VRPawn` 中剥离。

### 1.1 创建通用角色基类
- [ ] **创建 `BP_BaseCharacter`**
    - 父类：`ACLM_Character` (C++)
    - 职能：作为 VR Pawn 和 PC Pawn 的共同父类。
    - 包含内容：通用的移动设置、Tag 设置、基础属性。

### 1.2 提取逻辑到组件 (Actor Components)
- [ ] **创建 `BPC_HealthComponent`**
    - 职能：管理生命值 (`PlayerHealth`)、受伤 (`PlayerHealthMinusOne`)、死亡逻辑、UI 刷新回调。
    - 迁移：从 `BP_VRPawn` 中迁移相关变量和函数。
- [ ] **创建 `BPC_InventoryComponent`**
    - 职能：管理背包状态 (`InBackPack`)、物品存储 (`BowInBackPack`)、弹药数量 (`ArrowNumber`)。
    - 迁移：从 `BP_VRPawn` 中迁移背包相关逻辑。
- [ ] **创建 `BPC_SkillComponent`**
    - 职能：管理法力值、技能解锁状态、技能冷却。
    - 注意：只包含数据和状态逻辑，不包含具体的释放表现（VR 绘画 vs PC 按键）。

### 1.3 重构 `BP_VRPawn`
- [ ] **修改父类**：将 `BP_VRPawn` 的父类从 `ACLM_Character` 修改为 `BP_BaseCharacter`。
- [ ] **挂载组件**：在 `BP_VRPawn` 上添加上述三个新组件。
- [ ] **逻辑替换**：将 `BP_VRPawn` 中原有的变量引用替换为对组件的调用。

---

## 第二阶段：抽象交互系统

目标：使游戏中的物体（弓箭、攀爬点、机关）能够被“非 VR 手柄”触发。

### 2.1 泛化交互接口
- [ ] **重构 `BPI_CustomGrabAndRelease`**
    - 当前问题：接口参数依赖 `BP_MyMotionController`。
    - 解决方案：修改接口参数为 `Actor Interactor` (交互发起者) 或新增接口 `BPI_Interactor`。
    - 任务：
        - 修改 `OnGrab` 和 `OnRelease` 的签名。
        - 更新所有实现了该接口的蓝图（`BP_ClimbableVolume`, `BP_Bow`, `BP_StasisPoint` 等）。

### 2.2 适配交互发起者
- [ ] **更新 `BP_MyMotionController`**
    - 确保它能适配新的接口签名（作为 VR 环境下的 Interactor）。

### 2.3 适配交互对象
- [ ] **重构 `BP_Bow`**
    - 分离“物理拉弓”逻辑。
    - 新增 `SimulatePull(float Strength)` 函数，允许通过非物理方式（如鼠标按键时长）控制拉弓力度。
- [ ] **重构 `BP_ClimbableVolume`**
    - 识别交互者类型。如果是 PC Pawn，则不使用手部位置计算偏移，而是使用简单的“吸附+垂直移动”逻辑。

---

## 第三阶段：实现 PC 模式

目标：构建一个功能完整的 PC 第一人称角色。

### 3.1 创建 PC 角色
- [ ] **创建 `BP_PCPawn`**
    - 父类：`BP_BaseCharacter`
    - 组件：
        - CameraComponent (FPS 视角)
        - `BPC_HealthComponent`
        - `BPC_InventoryComponent`
        - `BPC_SkillComponent`
    - 移动：配置 CharacterMovement 实现标准的 WASD 移动、跳跃、蹲伏。

### 3.2 实现 PC 交互
- [ ] **创建 `BPC_PCInteractor`**
    - 职能：每帧或按键时进行射线检测 (Line Trace)。
    - 逻辑：检测到实现了 `BPI_CustomGrabAndRelease` 的物体时，显示交互提示。按下 'E' 键调用接口的 `OnGrab`。

### 3.3 输入映射
- [ ] **配置 Enhanced Input**
    - 为 PC 模式创建新的 Input Mapping Context (IMC_PC)。
    - 映射 WASD、鼠标视角、鼠标左键（攻击/射击）、E键（交互）、I键（背包）、技能按键（1, 2, 3, 4）。

---

## 第四阶段：技能与机制适配

目标：让为 VR 设计的星图技能和攀爬机制在 PC 上也能通过替代方式体验。

### 4.1 技能系统适配
- [ ] **抽象技能输入层**
    - VR 实现：保留 `BP_DrawManager`，绘制成功后调用 `BPC_SkillComponent` 释放技能。
    - PC 实现：在 `BP_PCPawn` 中监听按键（如 Q, F 或 数字键），直接调用 `BPC_SkillComponent` 释放技能。
- [ ] **技能释放表现**
    - 鹰眼 (Eagle Eye)：通用，无需修改。
    - 传送/其他技能：可能需要为 PC 添加准星辅助瞄准。

### 4.2 攀爬系统适配
- [ ] **PC 攀爬逻辑**
    - 在 `BP_ClimbableVolume` 中添加对 PC 角色的特殊处理。
    - 当 PC 角色“抓取”梯子时，禁用重力，将移动输入 (W/S) 转换为垂直移动。

---

## 第五阶段：UI 与 优化

### 5.1 UI 适配
- [ ] **创建 `BP_UIManager`**
    - 职能：根据当前的游戏模式（VR/PC）决定 UI 的显示方式。
    - VR：使用 WidgetComponent (3D UI)。
    - PC：使用 AddToViewport (2D UI)。

### 5.2 测试与调试
- [ ] **配置 GameMode**
    - 允许在编辑器中轻松切换 Default Pawn Class (`BP_VRPawn` <-> `BP_PCPawn`)。
- [ ] **全流程测试**
    - 确保 PC 模式下能通关（潜行、战斗、解谜）。
