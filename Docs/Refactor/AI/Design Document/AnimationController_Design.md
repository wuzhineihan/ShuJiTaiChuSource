# 动画控制器设计文档

## 目录

- [1. 概述](#1-概述)
- [2. 职责范围](#2-职责范围)
  - [核心职责](#21-核心职责)
  - [不负责的事项](#22-不负责的事项)
- [3. 架构设计](#3-架构设计)
  - [组件定位](#31-组件定位)
  - [数据流](#32-数据流)
- [4. 组件设计](#4-组件设计)
  - [UAnimationController](#41-uanimationcontroller)
- [5. 工作流程](#5-工作流程)
  - [初始化流程](#51-初始化流程)
  - [攻击动画流程](#52-攻击动画流程)
  - [武器切换流程](#53-武器切换流程)
- [6. 数据配置](#6-数据配置)
  - [蒙太奇映射表](#61-蒙太奇映射表)
  - [常见蒙太奇](#62-常见蒙太奇)
- [7. 设计特点](#7-设计特点)
  - [解耦性](#71-解耦性)
  - [复用性](#72-复用性)
  - [可扩展性](#73-可扩展性)
- [8. 与其他系统的交互](#8-与其他系统的交互)
  - [与武器系统的交互](#81-与武器系统的交互)
  - [与动画蒙太奇的交互](#82-与动画蒙太奇的交互)
  - [与骨架网格体的交互](#83-与骨架网格体的交互)
- [9. 扩展方向](#9-扩展方向)
  - [动画完成通知](#91-动画完成通知)
  - [动画中断处理](#92-动画中断处理)
  - [分层动画](#93-分层动画)
- [10. 调试支持](#10-调试支持)
  - [可视化信息](#101-可视化信息)
  - [日志记录](#102-日志记录)
- [11. 设计清单](#11-设计清单)

---

## 1. 概述

AnimationController 是一个独立的动画执行组件，负责接收武器系统的攻击请求并播放对应的动画蒙太奇。

**核心设计理念**：完全解耦动画逻辑与武器逻辑，使得动画系统不需要知道任何武器类型信息。

---

## 2. 职责范围

### 2.1 核心职责

- 接收来自武器系统的攻击请求事件（FAttackRequestedEvent）
- 根据攻击名称查找对应的蒙太奇资源
- 播放和管理动画蒙太奇的生命周期
- 与骨架网格体的 AnimInstance 直接交互

### 2.2 不负责的事项

- 任何武器逻辑判断
- 伤害计算
- 碰撞检测
- 弹药管理
- 任何游戏业务逻辑

---

## 3. 架构设计

### 3.1 组件定位

AnimationController 在 EnemyBase 中的角色：

```
EnemyBase (Pawn)
├── EventBusComponent (事件中转)
├── WeaponSlots + CurrentWeapon (武器管理)
└── AnimationController (动画执行) ← 纯粹的表现层
```

特点：
- 作为 EnemyBase 的子组件
- 通过事件总线接收事件（不直接依赖武器）
- 直接访问 Owner 的骨架网格体

### 3.2 数据流

```
武器组件 → FAttackRequestedEvent → EventBus
                                      ↓
                            AnimationController
                                      ↓
                          查找蒙太奇 → 播放动画
```

---

## 4. 组件设计

### 4.1 UAnimationController

**职责**：动画资源管理和蒙太奇播放

**关键数据**：
- AttackMontages：映射表
  - 键：攻击名称 (FName，如 "Light", "Heavy", "Charge")
  - 值：动画蒙太奇资源 (UAnimMontage*)
- CurrentMontage：当前正在播放的蒙太奇
- SwitchWeaponMontage：武器切换时的过渡动画

**关键行为**：

| 行为 | 说明 |
|------|------|
| OnAttackRequested | 接收攻击请求事件并处理 |
| PlayMontage | 播放指定蒙太奇 |
| StopMontage | 停止当前蒙太奇 |
| OnWeaponChanged | 武器切换时的动画过渡 |

**订阅的事件**：
- FAttackRequestedEvent：来自武器组件的攻击请求
- FWeaponChangedEvent：来自武器槽的武器切换通知

**发送的事件**：
- （暂无，或可扩展为动画完成事件）

---

## 5. 工作流程

### 5.1 初始化流程

1. EnemyBase::BeginPlay() 时创建 AnimationController 组件
2. AnimationController 向 EventBusComponent 订阅事件
3. 准备就绪，等待事件

### 5.2 攻击动画流程

1. **事件接收**：
   - AnimationController 收到 FAttackRequestedEvent
   - 事件包含 AttackName（如 "Light"）

2. **资源查找**：
   - 在 AttackMontages 中查找 AttackName
   - 若存在，获取蒙太奇资源
   - 若不存在，记录警告

3. **蒙太奇播放**：
   - 停止当前蒙太奇（若有）
   - 通过 AnimInstance->Montage_Play() 播放新蒙太奇
   - 记录当前播放的蒙太奇

4. **生命周期管理**：
   - 蒙太奇播放过程中触发 AnimNotify
   - AnimNotify 向武器系统发送 FWeaponNotifyEvent
   - 蒙太奇播放结束，清理状态

### 5.3 武器切换流程

1. **事件接收**：
   - AnimationController 收到 FWeaponChangedEvent
   - 事件包含新旧武器信息和切换时长

2. **过渡动画**：
   - 播放 SwitchWeaponMontage（如果配置了）
   - 按指定时长播放
   - 完成后，新武器的动画映射表已准备好

---

## 6. 数据配置

### 6.1 蒙太奇映射表

**在 EnemyConfigPreset 中定义**：

- AttackMontages：TMap<FName, UAnimMontage*>
  - 包含所有可能的攻击蒙太奇
  - 键应与武器系统发送的 AttackName 一致

**使用场景**：
- 不同敌人可能有不同的攻击蒙太奇
- 即使武器相同，动画效果可能不同（身材、风格等）

### 6.2 常见蒙太奇

| 蒙太奇名 | 说明 | 用于 |
|---------|------|------|
| Light | 轻攻击 | 所有武器 |
| Heavy | 重攻击 | 所有武器 |
| Special | 特殊攻击 | 特定武器 |
| Charge | 蓄力攻击 | 远程武器 |
| SwitchWeapon | 武器切换过渡 | 武器切换 |

---

## 7. 设计特点

### 7.1 解耦性

- 不知道武器的具体类型
- 不知道伤害计算细节
- 不知道碰撞检测
- 完全独立的表现层

### 7.2 复用性

- 相同的 AnimationController 可用于多种敌人
- 只需修改配置中的蒙太奇映射
- 可用于玩家角色

### 7.3 可扩展性

- 新增攻击类型只需添加蒙太奇和 FName
- 无需修改 AnimationController 代码
- 支持动态蒙太奇切换

---

## 8. 与其他系统的交互

### 8.1 与武器系统的交互

```
武器组件 (发送)
  ↓
FAttackRequestedEvent
  ↓
EventBusComponent (中转)
  ↓
AnimationController (接收)
  ↓
播放蒙太奇
  ↓
AnimNotify (发送)
  ↓
FWeaponNotifyEvent
  ↓
EventBusComponent (中转)
  ↓
武器组件 (接收)
```

### 8.2 与动画蒙太奇的交互

蒙太奇中的 AnimNotify 可以：
- 引用 EventBusComponent
- 发送 FWeaponNotifyEvent 到武器系统
- 传递精确的时序信息

### 8.3 与骨架网格体的交互

- 直接访问 Owner->GetMesh()->GetAnimInstance()
- 播放蒙太奇
- 获取当前播放进度
- 监听播放完成

---

## 9. 扩展方向

### 9.1 动画完成通知

可扩展为发送动画完成事件：
- FAnimationCompleteEvent
- 用于其他系统做最后收尾

### 9.2 动画中断处理

处理由于被打断需要立即中止当前动画的情况：
- 受到控制效果（击退、冻结等）
- 需要立即切换动画

### 9.3 分层动画

支持上身/下身分层蒙太奇：
- 独立控制上身攻击动画
- 下身可继续移动

---

## 10. 调试支持

### 10.1 可视化信息

- 当前播放蒙太奇名称
- 播放进度百分比
- 最后一次收到的事件

### 10.2 日志记录

- 收到 FAttackRequestedEvent 时记录
- 蒙太奇查找失败时记录警告
- 蒙太奇播放开始/结束时记录

---

## 11. 设计清单

- [ ] 创建 UAnimationController 基类
- [ ] 实现 OnAttackRequested 处理
- [ ] 实现 PlayMontage 播放逻辑
- [ ] 实现 OnWeaponChanged 过渡逻辑
- [ ] 配置蒙太奇映射表
- [ ] 创建 AnimNotify_WeaponAction（在 Weapon 文档中）
- [ ] 在蒙太奇中放置 AnimNotify
- [ ] 调试和优化
