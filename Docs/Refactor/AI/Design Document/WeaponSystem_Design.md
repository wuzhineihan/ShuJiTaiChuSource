# 武器系统设计文档

## 目录

- [武器系统设计文档](#武器系统设计文档)
  - [目录](#目录)
  - [1. 设计理念](#1-设计理念)
    - [核心原则](#核心原则)
    - [设计目标](#设计目标)
  - [2. 系统架构](#2-系统架构)
    - [架构分层](#架构分层)
  - [3. 核心组件](#3-核心组件)
    - [3.1 UWeaponComponentBase（武器基类）](#31-uweaponcomponentbase武器基类)
    - [3.2 UMeleeWeaponComponent（近战武器）](#32-umeleeweaponcomponent近战武器)
    - [3.3 URangedWeaponComponent（远程武器）](#33-urangedweaponcomponent远程武器)
    - [3.4 UAnimationController（动画控制器）](#34-uanimationcontroller动画控制器)
    - [3.5 UWeaponSlotsComponent（武器槽组件）](#35-uweaponslotscomponent武器槽组件)
  - [4. 事件系统定义](#4-事件系统定义)
    - [4.1 FAttackRequestedEvent](#41-fattackrequestedevent)
    - [4.2 FWeaponNotifyEvent](#42-fweaponnotifyevent)
    - [4.3 FWeaponChangedEvent](#43-fweaponchangedevent)
    - [4.4 FDamageDealtEvent](#44-fdamagedealtevent)
  - [5. 接口定义](#5-接口定义)
    - [IAttackable（可攻击接口）](#iattackable可攻击接口)
  - [6. 组件间通信策略](#6-组件间通信策略)
    - [直接引用（父子关系）](#直接引用父子关系)
    - [敌人与武器的交互](#敌人与武器的交互)
    - [事件总线通信（跨组件）](#事件总线通信跨组件)
  - [7. 数据配置](#7-数据配置)
    - [武器配置结构 (FWeaponConfig)](#武器配置结构-fweaponconfig)
    - [武器动画配置 (FWeaponAnimationConfig)](#武器动画配置-fweaponanimationconfig)
    - [敌人配置预设 (UEnemyConfigPreset DataAsset)](#敌人配置预设-uenemyconfigpreset-dataasset)
  - [8. 工作流程](#8-工作流程)
    - [初始化流程](#初始化流程)
    - [攻击流程](#攻击流程)
    - [武器切换流程](#武器切换流程)
  - [9. AnimNotify 使用规范](#9-animnotify-使用规范)
    - [自定义 AnimNotify\_WeaponAction](#自定义-animnotify_weaponaction)
    - [ActionTag 命名约定](#actiontag-命名约定)
    - [蒙太奇构建示例](#蒙太奇构建示例)
  - [10. 扩展性考虑](#10-扩展性考虑)
    - [新增武器类型](#新增武器类型)
    - [新增攻击类型（同武器类型内）](#新增攻击类型同武器类型内)
    - [玩家使用相同武器系统](#玩家使用相同武器系统)
  - [11. 性能优化建议](#11-性能优化建议)
    - [武器池化](#武器池化)
    - [事件过滤](#事件过滤)
    - [碰撞优化（近战）](#碰撞优化近战)
  - [12. 调试支持](#12-调试支持)
    - [可视化调试](#可视化调试)
    - [日志记录](#日志记录)
  - [附录：计划的文件结构](#附录计划的文件结构)
    - [Public 目录](#public-目录)
    - [Private 目录](#private-目录)
  - [设计决策总结](#设计决策总结)

---

## 1. 设计理念

### 核心原则

| 原则 | 说明 |
|------|------|
| **武器槽化** | 敌人可装备多个武器，运行时切换 |
| **业务与表现分离** | 武器逻辑与动画系统解耦 |
| **事件驱动** | 跨组件通信使用事件总线 |
| **数据驱动** | 武器配置通过 DataAsset 定义 |
| **接口抽象** | 统一的攻击接口，支持多态 |

### 设计目标

- 敌人可装备多种武器（近战、远程）
- 运行时动态切换武器
- 武器攻击触发动画、音效、特效
- 不同武器有不同的攻击方式
- 玩家也可以使用相同的武器系统

---

## 2. 系统架构

### 架构分层

EnemyBase 包含以下组件结构：

```
EnemyBase (Pawn)
├── EventBusComponent (事件中转)
├── WeaponSlotsComponent (武器管理) ← 专用组件
│   ├── 管理所有武器槽
│   ├── 处理武器创建和初始化
│   ├── 处理武器切换
│   └── 提供当前武器接口
├── AnimationController (动画执行)
└── 其他组件
```

**事件总线层**：EventBusComponent 作为中心枢纽
- 分发所有内部事件（攻击请求、动画通知、武器切换）
- 所有组件通过事件总线进行跨组件通信

**武器管理层**：WeaponSlotsComponent 作为专用组件
- 创建和初始化所有武器实例
- 管理武器槽（TMap 存储）
- 处理武器切换逻辑
- 提供当前武器查询接口
- 维护武器的生命周期

**动画执行层**：AnimationController
- 独立组件，完全解耦武器逻辑
- 监听攻击请求事件并播放蒙太奇
- 管理所有动画资源

---

## 3. 核心组件

### 3.1 UWeaponComponentBase（武器基类）

**职责**：
- 定义武器的通用接口
- 管理武器状态（是否攻击中、冷却等）
- 发送攻击请求事件到事件总线
- 监听动画通知事件并响应

**关键状态**：
- 是否正在攻击（bIsAttacking）
- 攻击冷却计时器（AttackCooldown）
- 基础伤害值（Damage）
- 攻击范围（AttackRange）
- 武器类型（EWeaponType）

**关键行为**：
- BeginAttack：发起攻击，发送事件
- CanAttack：检查是否满足攻击条件
- OnAnimNotifyReceived：处理来自 AnimNotify 的事件
- OnAttackBegin/OnAttackEnd：攻击生命周期回调

**实现的接口**：IAttackable, IEventBusSubscriber

**发送的事件**：
- FAttackRequestedEvent - 请求动画播放
- FAttackStartedEvent - 攻击已开始
- FAttackEndedEvent - 攻击已结束

**订阅的事件**：FWeaponNotifyEvent - 动画通知

---

### 3.2 UMeleeWeaponComponent（近战武器）

**职责**：
- 处理近战攻击逻辑
- 管理碰撞检测生命周期
- 处理连击系统

**特有功能**：
- 碰撞启用/禁用（通过 AnimNotify 控制时机）
- 单帧伤害判定（避免重复伤害）
- 连击计数和窗口管理
- 击中反馈（击退、僵直等）

**监听的 AnimNotify**：
- Weapon.CollisionOn - 启用碰撞检测
- Weapon.CollisionOff - 禁用碰撞检测
- Weapon.DealDamage - 造成伤害判定点
- Weapon.ComboWindow - 连击输入窗口

---

### 3.3 URangedWeaponComponent（远程武器）

**职责**：
- 处理远程攻击逻辑（弓箭、枪等）
- 管理弹药库存
- 处理瞄准和射击流程

**特有功能**：
- 瞄准状态管理（BeginAim/EndAim）
- 拉弦力度积累（DrawStrength 0-1）
- 发射物生成和配置
- 装填流程管理
- 弹药消耗和检查

**监听的 AnimNotify**：
- Weapon.BeginAim - 瞄准开始
- Weapon.Fire - 发射时机点
- Weapon.Reload - 装填开始

---

### 3.4 UAnimationController（动画控制器）

**职责**：
- 接收攻击请求事件
- 根据攻击类型选择对应蒙太奇
- 播放和管理动画蒙太奇
- 完全不依赖武器系统细节

**设计特点**：
- 纯粹的表现层组件
- 不知道武器类型和伤害计算
- 可复用于多种敌人和玩家

**详细设计**：详见 [AnimationController_Design.md](AnimationController_Design.md)

---

### 3.5 UWeaponSlotsComponent（武器槽组件）

**职责**：
- 创建和初始化所有武器实例
- 管理武器槽（TMap 存储）
- 处理武器切换逻辑
- 提供当前武器接询接口
- 向事件总线发送武器相关事件

**核心数据**：
- WeaponSlots：TMap<EWeaponType, UWeaponComponentBase*>
  - 存储所有创建的武器实例
  - 键：武器类型
  - 值：武器组件指针
- CurrentWeapon：当前激活的武器指针

**关键方法**：
- InitializeWeapons：初始化所有武器（从配置读取）
- SwitchWeapon：切换到指定类型的武器
- GetCurrentWeapon：获取当前武器
- GetWeapon：按类型获取武器
- HasWeapon：检查是否拥有指定类型的武器

**初始化流程**：

1. EnemyBase 创建 WeaponSlotsComponent
2. 调用 InitializeWeapons(ConfigPreset)
3. 组件读取 AvailableWeapons 列表
4. 对每个配置创建动态武器组件
5. 根据 AttachSocket 挂载到骨架
6. 初始化武器参数（伤害、范围、冷却等）
7. 存入 WeaponSlots 映射表
8. 设置 CurrentWeapon 指向 DefaultWeapon
9. 发送 FWeaponChangedEvent 通知其他系统

**切换流程**：

1. 敌人或 Controller 调用 SwitchWeapon(TargetType)
2. 检查 WeaponSlots[TargetType] 是否存在
3. 若不存在则创建新武器，若已存在则直接使用
4. 更新 CurrentWeapon = WeaponSlots[TargetType]
5. 发送 FWeaponChangedEvent 到事件总线
6. 其他系统（AnimationController 等）响应事件

**查询接口**：

敌人与武器的交互都通过以下接口：
- GetCurrentWeapon()：获取当前激活武器
- HasWeapon(Type)：检查是否拥有该类型武器
- GetWeapon(Type)：获取指定类型武器

**性能特性**：

- **武器池化**：所有武器在初始化时就已创建，避免运行时频繁分配
- **指针切换**：O(1) 的切换时间
- **内存占用**：敌人装备的所有武器都驻留在内存

**设计优势**：

- 职责清晰：所有武器管理逻辑集中在此组件
- 接口简洁：EnemyBase 只需调用少量接口
- 易于扩展：新增武器功能只需修改此组件
- 独立组件：可独立单元测试

---



## 4. 事件系统定义

### 4.1 FAttackRequestedEvent

**用途**：武器组件请求动画系统播放攻击蒙太奇

**包含信息**：
- Weapon：发起攻击的武器引用
- AttackName：攻击类型标识（如 "Light", "Heavy", "Special"）
- Target：攻击目标（可选，供 AI 使用）
- Timestamp：事件发送时刻

**发送者**：UWeaponComponentBase  
**监听者**：UAnimationController

---

### 4.2 FWeaponNotifyEvent

**用途**：动画蒙太奇中的 AnimNotify 向武器系统发送时序事件

**包含信息**：
- ActionTag：动作类型标识（GameplayTag 格式，如 "Weapon.Fire"）
- Source：事件源 Actor
- Timestamp：事件发送时刻
- Parameters：可选的浮点数参数

**标签约定**：
- Weapon.BeginAttack - 攻击阶段开始
- Weapon.CollisionOn - 启用碰撞检测（近战）
- Weapon.CollisionOff - 禁用碰撞检测（近战）
- Weapon.DealDamage - 进行伤害判定（近战）
- Weapon.Fire - 发射时机（远程）
- Weapon.BeginAim - 瞄准开始（远程）
- Weapon.Reload - 装填开始（远程）
- Weapon.ComboWindow - 连击输入窗口开启（近战）
- Weapon.EndAttack - 攻击阶段结束

**发送者**：UAnimNotify_WeaponAction（蒙太奇中的自定义 AnimNotify）  
**监听者**：对应的武器组件

---

### 4.3 FWeaponChangedEvent

**用途**：通知所有系统武器已切换

**包含信息**：
- NewWeapon：新激活的武器引用
- OldWeapon：前一个武器引用
- WeaponType：新武器的类型
- SwitchDuration：切换过渡时间（默认 0.5 秒）

**发送者**：AEnemyBase（武器槽管理）  
**监听者**：AnimationController, UI 系统, EnemyState, 视觉反馈系统等

---

### 4.4 FDamageDealtEvent

**用途**：武器造成伤害时通知相关系统

**包含信息**：
- Weapon：造成伤害的武器
- DamagedActor：受伤的目标
- Damage：伤害值
- HitLocation：击中的世界位置
- HitDirection：击中的方向向量

**发送者**：UWeaponComponentBase（武器组件）  
**监听者**：音效系统, 特效系统, 统计系统, UI 系统等

---

## 5. 接口定义

### IAttackable（可攻击接口）

**方法**：
- CanAttack() → bool：检查是否可以发起攻击
- BeginAttack(FName) → void：开始指定类型的攻击
- EndAttack() → void：结束当前攻击
- GetAttackRange() → float：获取攻击有效范围
- GetAttackType() → EAttackType：获取武器的攻击类型分类

**实现者**：所有武器组件（Melee, Ranged, Special）

---

## 6. 组件间通信策略

### 直接引用（父子关系）

用于**有明确所有权**的关系：

**EnemyBase** → 直接持有
- WeaponSlotsComponent：武器槽管理组件
- AnimationController：动画控制组件

**WeaponSlotsComponent** → 创建和管理
- WeaponSlots[Type]：所有武器组件
- 通过 GetOwner() 访问 EnemyBase 和其骨架网格体

**武器组件** → GetOwner() 获得 EnemyBase
- 访问骨架网格体（用于骨骼挂载）
- 访问事件总线（用于发送事件）
- 访问库存组件（远程武器获取弹药）

**AnimationController** → GetOwner() 获得 EnemyBase
- 访问骨架网格体的 AnimInstance（用于播放蒙太奇）

### 敌人与武器的交互

所有敌人与武器的交互都通过 **WeaponSlotsComponent** 代理：

```
EnemyBase / Controller
     ↓
调用 GetWeapon / SwitchWeapon
     ↓
WeaponSlotsComponent
     ↓
返回 CurrentWeapon 或切换武器
     ↓
与具体武器交互
```

**优势**：
- 敌人无需知道武器细节
- 所有武器管理逻辑集中
- 便于添加额外的武器管理功能

### 事件总线通信（跨组件）

用于**解耦的组件交互**：

**发送方** → 武器组件、AnimNotify
- 发送 FAttackRequestedEvent
- 发送 FWeaponNotifyEvent
- 发送 FDamageDealtEvent

**接收方** → 武器组件、AnimationController
- 订阅 FWeaponNotifyEvent
- 订阅 FAttackRequestedEvent

---

## 7. 数据配置

### 武器配置结构 (FWeaponConfig)

**用途**：定义单个武器的所有参数

**包含字段**：
- WeaponClass：TSubclassOf<UWeaponComponentBase>，要创建的组件类
- WeaponMesh：TSoftObjectPtr<USkeletalMesh>，武器 3D 模型（软引用延迟加载）
- AttachSocket：挂载点名称（如 "weapon_r", "arrow_nock"）
- BaseDamage：基础伤害值（float）
- AttackRange：攻击有效范围（float）
- AttackCooldown：攻击间隔时间（float）

### 武器动画配置 (FWeaponAnimationConfig)

**用途**：定义武器的动画资源映射

**包含字段**：
- AttackMontages：TMap<FName, UAnimMontage*>
  - 键：攻击名称（如 "Light", "Heavy", "Charge"）
  - 值：对应的蒙太奇资源
- SwitchWeaponMontage：武器切换时播放的蒙太奇

### 敌人配置预设 (UEnemyConfigPreset DataAsset)

**用途**：在 DataAsset 中定义单个敌人的完整武器配置

**包含字段**：
- AvailableWeapons：TArray<FWeaponConfig>，敌人可用的所有武器列表
- WeaponAnimations：FWeaponAnimationConfig，全局动画映射
- DefaultWeapon：EWeaponType，初始激活的武器类型

---

## 8. 工作流程

### 初始化流程

1. EnemyBase::BeginPlay()
2. 创建 WeaponSlotsComponent
3. 调用 WeaponSlotsComponent->InitializeWeapons(ConfigPreset)
4. 组件读取 AvailableWeapons 列表
5. 遍历每个武器配置
6. 创建对应的动态组件实例
7. 根据 AttachSocket 挂载到骨架
8. 初始化每个武器的参数（伤害、范围、冷却等）
9. 将组件存入 WeaponSlots Map（按武器类型索引）
10. 设置 CurrentWeapon 指向 DefaultWeapon
11. 发送 FWeaponChangedEvent 通知其他系统
12. AnimationController 向事件总线订阅 FAttackRequestedEvent

### 攻击流程

1. **决策阶段**：
   - Controller 通过 EnemyState 检查是否可以攻击
   - 调用 WeaponSlotsComponent->GetCurrentWeapon()->BeginAttack("Light")

2. **请求阶段**：
   - 武器组件检查 CanAttack()（冷却、状态等）
   - 发送 FAttackRequestedEvent 到事件总线
   - 标记 bIsAttacking = true

3. **动画播放**：
   - AnimationController 接收事件
   - 在 AttackMontages 中查找对应蒙太奇
   - 通过 AnimInstance 播放蒙太奇

4. **时序执行**：
   - 蒙太奇播放过程中，在特定帧触发 AnimNotify
   - AnimNotify_WeaponAction 发送 FWeaponNotifyEvent（如 "Weapon.Fire"）

5. **业务逻辑**：
   - 武器组件监听并接收事件
   - 根据 ActionTag 执行对应逻辑：
     - 远程：生成发射物
     - 近战：启用碰撞检测
   - 发送 FDamageDealtEvent

6. **反馈系统**：
   - 音效系统播放声音
   - 特效系统播放视觉效果
   - 伤害系统应用伤害

### 武器切换流程

1. **触发切换**：
   - Controller 调用 WeaponSlotsComponent->SwitchWeapon(EWeaponType::Ranged)

2. **检查与创建**：
   - WeaponSlotsComponent 检查 WeaponSlots 是否已有该类型武器
   - 若无则创建对应的动态组件
   - 若已有则复用现有实例

3. **更新指针**：
   - 保存 OldWeapon = CurrentWeapon
   - 设置 CurrentWeapon = WeaponSlots[TargetType]

4. **事件通知**：
   - WeaponSlotsComponent 发送 FWeaponChangedEvent 到事件总线
   - 传递新旧武器和切换时长

5. **系统响应**：
   - AnimationController 播放切换蒙太奇
   - UI 系统更新武器显示
   - EnemyState 更新可用能力

---

## 9. AnimNotify 使用规范

### 自定义 AnimNotify_WeaponAction

**用途**：在蒙太奇的特定帧发送事件给武器系统

**工作机制**：
- 在蒙太奇编辑器中放置此 Notify
- 设置 ActionTag（GameplayTag 类型）
- 在指定帧触发时，发送 FWeaponNotifyEvent 到事件总线

### ActionTag 命名约定

所有 ActionTag 以 "Weapon." 前缀开头，便于事件过滤和识别

**常用标签**：
- Weapon.BeginAttack - 攻击动画开始
- Weapon.CollisionOn - 启用碰撞（近战）
- Weapon.CollisionOff - 禁用碰撞（近战）
- Weapon.DealDamage - 进行伤害判定（近战）
- Weapon.Fire - 发射时刻（远程）
- Weapon.BeginAim - 瞄准开始（远程）
- Weapon.EndAim - 瞄准结束（远程）
- Weapon.Reload - 装填开始（远程）
- Weapon.ComboWindow - 连击窗口打开（近战）
- Weapon.EndAttack - 攻击动画结束

### 蒙太奇构建示例

**近战斩击蒙太奇** (AM_MeleeSlash)：
- 0.1s：BeginAttack 通知 → 武器初始化
- 0.2s：CollisionOn 通知 → 启用碰撞盒
- 0.4s：DealDamage 通知 → 进行伤害判定
- 0.6s：CollisionOff 通知 → 禁用碰撞盒
- 0.8s：EndAttack 通知 → 攻击结束
- 0.9s：ComboWindow 通知 → 打开连击输入窗口（0.3 秒）

**弓箭射击蒙太奇** (AM_BowShoot)：
- 0.2s：BeginAim 通知 → 进入瞄准状态
- 0.5s：Fire 通知 → 发射箭矢（此时 DrawStrength 已积累）
- 0.8s：EndAttack 通知 → 攻击完全结束

---

## 10. 扩展性考虑

### 新增武器类型

**步骤**：
1. 创建新武器组件类（继承 UWeaponComponentBase）
2. 实现特定的 OnAnimNotifyReceived 逻辑
3. 在 DataAsset 中添加 FWeaponConfig 配置
4. 创建该武器的动画蒙太奇和 AnimNotify

**无需修改**：事件总线、AnimationController、其他系统

### 新增攻击类型（同武器类型内）

**步骤**：
1. 在 AttackMontages 中添加新蒙太奇（新的 FName）
2. 在蒙太奇中添加相应的 AnimNotify 和 ActionTag
3. 在武器组件中处理新的 ActionTag（若需特殊逻辑）

**无需修改**：事件定义、通信机制

### 玩家使用相同武器系统

**设计优势**：
- 玩家 Controller 也能直接调用 Weapon->BeginAttack()
- 共享相同的事件总线和动画系统
- 只需替换 Controller，武器系统无需任何改动

---

## 11. 性能优化建议

### 武器池化

策略：在 EnemyBase 初始化时预先创建所有可能的武器，存入 WeaponSlots

优势：避免运行时频繁的组件创建和销毁，提高频繁切换的流畅度

### 事件过滤

策略：武器组件只订阅自己关心的 ActionTag，使用 GameplayTag 的层级过滤

优势：减少不相关事件的回调，降低 CPU 开销

### 碰撞优化（近战）

策略：
- 仅在 CollisionOn/Off 时刻启用/禁用碰撞盒（不是每帧）
- 使用简单碰撞体（Box/Capsule，不用网格体碰撞）
- 单帧伤害判定，避免同帧多次判定

优势：大幅降低碰撞检测的性能开销

---

## 12. 调试支持

### 可视化调试

- 显示武器槽状态（当前激活哪个武器）
- 显示事件流（发送和接收的事件及时间）
- 显示碰撞检测的活跃范围和命中信息
- 显示攻击冷却倒计时进度

### 日志记录

关键事件记录：
- 攻击请求：Attack Requested with Type={AttackName}
- AnimNotify 接收：AnimNotify Received with Tag={ActionTag}
- 武器切换：Weapon Switched from {OldType} to {NewType}
- 伤害输出：Damage Dealt by {WeaponType} = {DamageValue}

---

## 附录：计划的文件结构

### Public 目录
- Weapon/WeaponTypes.h - 枚举（EWeaponType, EAttackType）和结构体定义
- Weapon/WeaponComponentBase.h - 武器基类声明
- Weapon/MeleeWeaponComponent.h - 近战武器组件
- Weapon/RangedWeaponComponent.h - 远程武器组件
- Weapon/AnimationController.h - 动画控制器
- Weapon/Events/ - 所有事件结构体声明
- Weapon/Interfaces/IAttackable.h - 攻击能力接口
- Weapon/AnimNotify/AnimNotify_WeaponAction.h - 自定义 AnimNotify 声明

### Private 目录
- Weapon/ - 对应的实现文件（.cpp）

---

## 设计决策总结

| 决策 | 理由 |
|------|------|
| 使用事件总线 | 解耦跨组件通信，支持灵活的系统扩展 |
| 武器槽使用 TMap | O(1) 快速查找，支持运行时切换 |
| AnimationController 独立 | 动画完全不依赖武器类型，复用性强 |
| AnimNotify + GameplayTag | 精确的时序控制，标签化便于过滤和调试 |
| 数据驱动配置 | 敌人无需代码改动就能修改武器组合 |
| 混合通信策略 | 事件用于解耦，接口用于明确依赖 |
