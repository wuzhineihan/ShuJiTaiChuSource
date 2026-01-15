# 定身术技能实现总结

## 实现日期
2026-01-16

## 概述
成功将蓝图定身术系统重构为 C++ 实现，通过技能策略模式+抓取系统实现 PC/VR 统一的定身球发射逻辑。

---

## 实现的功能

### 1. 创建通用工具类 `UGameUtils`
**文件位置：**
- `Source/VRTest/Public/Tools/GameUtils.h`
- `Source/VRTest/Private/Tools/GameUtils.cpp`

**功能：**
- 提供 `FindAngleClosestGrabbableTarget` 静态函数
- 在锥形范围内查找角度最近的可抓取目标
- 支持可选的重力手套兼容性检查
- 复用于 VR 重力手套、PC 定身术、以及未来可能的其他系统

**重构优化：**
- 重构了 `VRGrabHand::FindAngleClosestTarget()` 以使用新的工具函数
- 避免代码重复，提高可维护性

---

### 2. 在 `UPlayerGrabHand` 中添加 GrabLock 机制
**修改文件：**
- `Source/VRTest/Public/Grabber/PlayerGrabHand.h`
- `Source/VRTest/Private/Grabber/PlayerGrabHand.cpp`

**新增内容：**
- 添加 `bGrabLocked` 成员变量（BlueprintReadOnly）
- 添加 `SetGrabLock(bool bLock)` 公共接口函数
- 在 `TryGrab()` 和 `TryRelease()` 开头检查锁状态
- `GrabObject()` 和 `ReleaseObject()` 不受锁限制（供技能系统直接调用）

**用途：**
- 技能系统在生成定身球并抓取后锁定手部
- 防止玩家在技能执行过程中意外释放定身球
- 投掷/发射后解锁手部

---

### 3. 完善 `AStasisPoint` 的 `IGrabbable` 接口实现
**修改文件：**
- `Source/VRTest/Public/Skill/Stasis/StasisPoint.h`
- `Source/VRTest/Private/Skill/Stasis/StasisPoint.cpp`

**实现的接口函数：**
- `GetGrabType_Implementation()` → 返回 `EGrabType::Custom`
- `GetGrabPrimitive_Implementation()` → 返回 `Sphere` 组件
- `CanBeGrabbedBy_Implementation()` → 返回 `true`
- `CanBeGrabbedByGravityGlove_Implementation()` → 返回 `false`
- `SupportsDualHandGrab_Implementation()` → 返回 `false`
- `OnGrabbed_Implementation()` → 设置 `Target = Hand`
- `OnReleased_Implementation()` → 设置 `Target = nullptr`
- `OnGrabSelected_Implementation()` → 空实现
- `OnGrabDeselected_Implementation()` → 空实现

**设计要点：**
- Custom 类型避免 PhysicsHandle 干扰，StasisPoint 使用自己的 FakePhysics 逻辑
- OnReleased 设置 Target 为 nullptr，停止跟随手部

---

### 4. ��建定身术技能策略类 `AStasisSkillStrategy`
**文件位置：**
- `Source/VRTest/Public/Skill/Stasis/StasisSkillStrategy.h`
- `Source/VRTest/Private/Skill/Stasis/StasisSkillStrategy.cpp`

**实现逻辑：**
1. 根据 `FSkillContext::bIsRightHand` 确定使用哪只手
2. 检查手部是否已持有物体
3. 在手部位置 Spawn `AStasisPoint`
4. 调用手部 `GrabObject()` 直接抓取定身球
5. 调用 `SetGrabLock(true)` 锁定手部

**配置：**
- `StasisPointClass`：可在蓝图中配置的定身球类（默认 `AStasisPoint`）

**后续流程：**
- PC：等待玩家投掷操作（在 `ABasePCPlayer::TryThrow` 中处理）
- VR：等待手部速度检测触发发射（需要在 VR 侧实现）

---

### 5. 扩展 PC 投掷逻辑支持 StasisPoint
**修改文件：**
- `Source/VRTest/Public/Game/BasePCPlayer.h`
- `Source/VRTest/Private/Game/BasePCPlayer.cpp`

**新增配置属性：**
- `StasisFireSpeedScalar`：定���球发射速度倍数（默认 1000.0f）
- `StasisDetectionRadius`：定身球目标检测半径（默认 500.0f）
- `StasisDetectionAngle`：定身球目标检测最大角度（默认 30.0 度）

**新增函数：**
- `HandleStasisPointThrow(UPCGrabHand* ThrowHand, AStasisPoint* StasisPoint)`

**实现逻辑：**
1. 使用 `UGameUtils::FindAngleClosestGrabbableTarget()` 查找目标
2. 筛选实现了 `IStasisable` 接口的目标
3. 计算发射速度：`CameraForward * StasisFireSpeedScalar`
4. 调用 `ReleaseObject()` 释放定身球
5. 调用 `StasisPoint->Fire()` 发射
6. 调用 `SetGrabLock(false)` 解锁手部

**集成到 TryThrow：**
- 在 `TryThrow()` 函数开头添加 `AStasisPoint` 类型检查
- 如果是定身球，调用 `HandleStasisPointThrow()` 并返回

---

## VR 侧实现（待完成）

### 建议方案
VR 侧需要添加速度检测逻辑，可以选择以下任一方案：

**方案 A：在 VRGrabHand 中添加速度检测**
- 在 `Tick()` 中检测手部持有 `AStasisPoint` 且速度超过阈值
- 触发时调用类似 PC 的发射逻辑

**方案 B：创建 VR 定身球管理器（类似蓝图的 BP_VRStasisManager）**
- 继承自 Actor
- 在 `BeginPlay` 时由技能策略生成并绑定到手部
- 在 `Tick` 中检测速度并触发发射
- 发射后销毁自身

### VR 发射逻辑参考（基于蓝��）
```cpp
// 伪代码
void VRStasisManager::Tick(float DeltaTime)
{
    UpdateHandVelocity(DeltaTime);
    
    if (!bSpeedOverThreshold && CurrentSpeed > SpeedThreshold)
    {
        bSpeedOverThreshold = true;
    }
    
    if (bSpeedOverThreshold && CurrentSpeed < LastSpeed)
    {
        // 速度开始下降，触发发射
        Hand->ReleaseObject();
        
        AActor* Target = FindTargetToStasis(LastVelocity);
        FVector InitVelocity = LastVelocity * FireSpeedFactor;
        
        StasisPoint->Fire(InitVelocity, Target ? Target->GetRootComponent() : nullptr);
        Hand->SetGrabLock(false);
        
        Destroy();
    }
}
```

---

## 技能注册

定身术技能策略类已创建，需要在蓝图中注册：

**位置：** `Content/.../.../SkillAsset`（或对应的 DataAsset）

**注册方式：**
- 打开 SkillAsset DataAsset
- 在 `StrategyClassMap` 中添加：
  - Key: `ESkillType::Freeze`
  - Value: `BP_StasisSkillStrategy`（继承自 `AStasisSkillStrategy` 的蓝图类）

---

## 测试要点

### PC 端测试
1. 学习定身术技能
2. 绘制定身术星图
3. 确认定身球生成在绘制手位置
4. 确认手部被锁定（无法 TryGrab/TryRelease）
5. 按投掷键发射定身球
6. 确认定身球朝摄像机前方飞行
7. 如果范围内有可定身目标，定身球应跟踪目标
8. 定身球击中目标后，目标进入定身状态
9. 确认手部解锁

### VR 端测试（实现后）
1. 学习定身术技能
2. 绘制定身术星图
3. 确认定身球生成在绘制手位置
4. 确认手部被锁定
5. 快速挥动手部（速度超过阈值）
6. 速度下降时，定身球自动发射
7. 确认定身球朝手部运动方向飞行
8. 确认目标跟踪和定身效果
9. 确认手部解锁

---

## 代码质量

### 编译状态
- ✅ 所有文件编译通过
- ⚠️ 仅有未使用变量警告（原有代码）
- ✅ 无严重错误或链接错误

### 设计优势
1. **职责分离清晰**：技能策略、抓取系统、工具函数各司其职
2. **代码复用**：`UGameUtils::FindAngleClosestGrabbableTarget` 可复用于多个系统
3. **扩展性强**：PC/VR 各自处理输入，但共享核心逻辑
4. **接口一致**：StasisPoint 完整实现 IGrabbable，与抓取系统无缝集成
5. **状态管理**：GrabLock 机制确保技能执行期间的手部状态安全

---

## 文件清单

### 新增文件
- `Source/VRTest/Public/Tools/GameUtils.h`
- `Source/VRTest/Private/Tools/GameUtils.cpp`
- `Source/VRTest/Public/Skill/Stasis/StasisSkillStrategy.h`
- `Source/VRTest/Private/Skill/Stasis/StasisSkillStrategy.cpp`

### 修改文件
- `Source/VRTest/Public/Grabber/PlayerGrabHand.h`
- `Source/VRTest/Private/Grabber/PlayerGrabHand.cpp`
- `Source/VRTest/Public/Skill/Stasis/StasisPoint.h`
- `Source/VRTest/Private/Skill/Stasis/StasisPoint.cpp`
- `Source/VRTest/Public/Game/BasePCPlayer.h`
- `Source/VRTest/Private/Game/BasePCPlayer.cpp`
- `Source/VRTest/Private/Grabber/VRGrabHand.cpp` (重构使用 GameUtils)

---

## 后续工作

1. **VR 侧实现**：添加手部速度检测和自动发射逻辑
2. **蓝图注册**：在 SkillAsset 中注册 Freeze → StasisSkillStrategy 映射
3. **测试验证**：完整测试 PC 和 VR 的定身术流程
4. **参数调优**：根据游戏体验调整检测半径、角度、速度等参数
5. **音效/特效**：添加定身球生成、飞行、命中的音效和特效
6. **UI 反馈**：添加技能触发、定身状态的 UI 提示

---

## 技术要点回顾

### Custom GrabType 的使用
- StasisPoint 使用 Custom 类型避免 PhysicsHandle 控制
- Custom 类型在 `GrabObject()` 中跳过物理抓取逻辑
- `HeldActor`/`bIsHolding` 状态仍然正确更新
- StasisPoint 通过自己的 FakePhysics 系统跟随 Target

### GrabLock 机制
- 只阻止 `TryGrab`/`TryRelease`（玩家输入）
- 不阻止 `GrabObject`/`ReleaseObject`（直接调用）
- 技能系统可以绕过锁直接操作手部

### 角度检测算法
- 使用球形 Overlap + 点积计算角度
- 筛选实现 IGrabbable 的 Actor
- 可选筛选实现 IStasisable 的 Actor
- 找到角度最小（最接近方向）的目标

---

## 参考文档
- `Docs/Refactor/skill.md` - 技能系统重构设计
- `BP2AIExport/VRGamePlay/Skill/Stasis/` - 原蓝图导出��档

