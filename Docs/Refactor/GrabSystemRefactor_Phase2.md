# 抓取系统重构 - 第二阶段

**创建日期**：2025-12-31
**状态**：待实现

---

## 一、重构目标

1. **简化抓取类型**：移除 Snap 类型，统一使用 PhysicsControl
2. **统一抓取方式**：WeaponSnap 也改用 PhysicsControl，不再使用 Attach
3. **弓箭系统封装**：弓仅暴露 IGrabbable 接口，PC/VR 统一通过抓取逻辑交互
4. **统一释放逻辑**：移除 ReleaseAttachOnly，统一使用 ReleaseObject
5. **自动弓弦交互**：通过碰撞检测自动触发搭箭和抓弓弦

---

## 二、EGrabType 变更

### 变更前
```cpp
enum class EGrabType : uint8
{
    None,        // 不可抓取
    Free,        // PhysicsControl 自由跟随
    Snap,        // PhysicsControl 对齐（待删除）
    WeaponSnap,  // Attach（待改为 PhysicsControl）
    HumanBody,   // PhysicsControl 骨骼控制
    Custom       // 自定义逻辑
};
```

### 变更后
```cpp
enum class EGrabType : uint8
{
    None,        // 不可抓取
    Free,        // PhysicsControl 自由跟随（保持抓取时相对位置）
    WeaponSnap,  // PhysicsControl 对齐到武器偏移（原 Attach 改为 PhysicsControl）
    HumanBody,   // PhysicsControl 骨骼控制
    Custom       // 自定义逻辑（如弓弦抓取）
};
```

---

## 三、详细修改清单

### 3.1 GrabTypes.h

| 修改项 | 说明 |
|-------|------|
| 移除 `Snap` | 从 EGrabType 枚举中删除 |

---

### 3.2 IGrabbable.h

| 修改项 | 说明 |
|-------|------|
| 移除 `GetSnapOffset()` | 删除接口方法声明 |

---

### 3.3 GrabbeeObject.h / .cpp

| 修改项 | 说明 |
|-------|------|
| 移除 `SnapOffset` 属性 | 删除 UPROPERTY |
| 移除 `GetSnapOffset_Implementation()` | 删除实现 |

---

### 3.4 PlayerGrabHand.h / .cpp

| 修改项 | 说明 |
|-------|------|
| 添加 `USphereComponent* HandCollision` | 手部碰撞体，用于检测与弓弦的 overlap |
| 移除 `ValidateGrab()` | 删除函数，在 GrabObject 开头统一检查 |
| 移除 `ReleaseAttachOnly()` | 删除函数，统一使用 ReleaseObject |
| 移除 `GrabSnap()` | 删除函数 |
| 修改 `GrabWeaponSnap()` | 改用 PhysicsControl 而非 Attach |
| 修改 `ReleaseObject()` | 移除 ReleaseAttach 调用，统一用 ReleasePhysicsControl |
| 修改 `GrabObject()` | 开头添加 CanBeGrabbedBy 检查，移除 Snap 分支 |
| 暴露 PhysicsControl 参数 | 添加 WeaponSnapStrength/Damping 等参数供蓝图调整 |

**HandCollision 配置**：
```cpp
HandCollision = CreateDefaultSubobject<USphereComponent>(TEXT("HandCollision"));
HandCollision->SetupAttachment(this);
HandCollision->SetSphereRadius(5.0f);
HandCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
HandCollision->SetCollisionResponseToAllChannels(ECR_Overlap);
HandCollision->ComponentTags.Add(FName("player_hand"));
```

**GrabObject 开头检查**：
```cpp
void UPlayerGrabHand::GrabObject(AActor* TargetActor, FName BoneName)
{
    if (!TargetActor) return;
    
    IGrabbable* Grabbable = Cast<IGrabbable>(TargetActor);
    if (!Grabbable) return;
    
    if (!IGrabbable::Execute_CanBeGrabbedBy(TargetActor, this)) return;
    
    // ... 继续原有逻辑
}
```

---

### 3.5 VRGrabHand.h / .cpp

| 修改项 | 说明 |
|-------|------|
| 确保 HandCollision 正确设置 | 继承自基类，可能需要调整半径 |

---

### 3.6 PCGrabHand.h / .cpp

| 修改项 | 说明 |
|-------|------|
| 确保 HandCollision 正确设置 | 继承自基类 |

---

### 3.7 BaseEnemy.h / .cpp

| 修改项 | 说明 |
|-------|------|
| 添加 `TSet<UPlayerGrabHand*> ControllingHands` | 追踪双手抓取状态 |
| 完善 `OnGrabbed_Implementation()` | 添加 `ControllingHands.Add(Hand)` |
| 完善 `OnReleased_Implementation()` | 添加 `ControllingHands.Remove(Hand)` |

---

### 3.8 Bow.h / .cpp

| 修改项 | 说明 |
|-------|------|
| 移除 `SetHandInStringArea()` | 完全依赖 overlap 检测 |
| 移除 `UpdateStringFromHandPosition()` | Tick 中统一处理 |
| 移除 `SetPullAmount()` | 不再需要程序化设置 |
| 移除 `bIsPCMode` | 已在之前移除，确认 |
| `StartPullingString()` 改为 private | 仅由 OnGrabbed 内部调用 |
| 修改 `OnStringCollisionBeginOverlap()` | 实现自动搭箭+抓弓弦逻辑 |
| 更新注释 | 移除过时的 PC/VR 模式说明 |

**OnStringCollisionBeginOverlap 新逻辑**：
```cpp
void ABow::OnStringCollisionBeginOverlap(...)
{
    // 检查是否是玩家的手
    if (!OtherComp->ComponentHasTag(FName("player_hand")))
        return;
    
    // 获取手组件
    UPlayerGrabHand* Hand = GetHandFromCollision(OtherComp);
    if (!Hand || Hand == BodyHoldingHand)
        return;
    
    // 弓身必须已被抓取
    if (!bBodyHeld)
        return;
    
    // 弓弦已被抓取则跳过
    if (bStringHeld)
        return;
    
    // 检查手是否持有箭
    if (AArrow* Arrow = Cast<AArrow>(Hand->HeldActor))
    {
        // 手释放箭
        Hand->ReleaseObject();
        // 搭箭
        NockArrow(Arrow);
    }
    
    // 手抓弓弦
    Hand->GrabObject(this);
}
```

**需要添加的辅助函数**：
```cpp
UPlayerGrabHand* ABow::GetHandFromCollision(UPrimitiveComponent* Comp) const;
```

---

### 3.9 BasePCPlayer.h / .cpp

| 修改项 | 说明 |
|-------|------|
| 移除 `StopDrawBow()` | 已弃用 |
| 修改 `StartDrawBow()` | 新流程：生成箭→抓箭→瞬移到弓弦→自动搭箭→interp到拉弓位置 |
| 修改 `ReleaseBowString()` | 移除 SetHandInStringArea(false) |
| 修改 `SetBowArmed(false)` | 确保 ReleaseObject 在 DestroyBow 之前 |
| 添加 `FTransform StringCollisionTransform` | 弓弦位置（蓝图配置） |
| 添加 `FTransform DrawingTransform` | 拉弓位置（蓝图配置） |

**StartDrawBow 新流程**：
```cpp
void ABasePCPlayer::StartDrawBow()
{
    if (!bIsAiming || !CurrentBow) return;
    
    // 检查是否有箭
    if (!InventoryComponent || !InventoryComponent->HasArrow())
    {
        PlayNoArrowSound();
        return;
    }
    
    bIsDrawingBow = true;
    
    // 消耗一支箭并生成
    InventoryComponent->ConsumeArrow();
    AArrow* Arrow = SpawnArrowAtHand();  // 在右手位置生成箭
    
    // 右手抓箭
    RightHand->GrabObject(Arrow);
    
    // 瞬移右手到弓弦位置（这会触发 overlap → 自动搭箭 + 抓弓弦）
    RightHand->SetWorldTransform(GetStringCollisionTransform());
    
    // Interp 到拉弓位置
    RightHand->InterpToTransform(DrawingRightHandTransform);
}
```

**ReleaseBowString 新流程**：
```cpp
void ABasePCPlayer::ReleaseBowString()
{
    if (!bIsDrawingBow) return;
    
    bIsDrawingBow = false;
    
    // 释放弓弦（触发 OnReleased → 发射箭）
    if (RightHand && RightHand->bIsHolding && RightHand->HeldActor == CurrentBow)
    {
        RightHand->ReleaseObject();
    }
    
    // 右手回到默认位置
    RightHand->InterpToDefaultTransform();
}
```

---

### 3.10 BaseVRPlayer.h / .cpp

| 修改项 | 说明 |
|-------|------|
| 修改 `SetBowArmed(false)` | 使用 `ReleaseObject()` 而非 `ReleaseAttachOnly()` |

**SetBowArmed 新流程**：
```cpp
void ABaseVRPlayer::SetBowArmed(bool bArmed)
{
    if (bIsBowArmed && !bArmed)
    {
        // 如果右手抓着弓弦，先释放（会触发发射）
        if (RightHand && RightHand->bIsHolding && RightHand->HeldActor == CurrentBow)
        {
            RightHand->ReleaseObject();
        }
        
        // 释放左手持有的弓
        if (LeftHand && LeftHand->bIsHolding && LeftHand->HeldActor == CurrentBow)
        {
            LeftHand->ReleaseObject();
        }
    }
    
    Super::SetBowArmed(bArmed);
    
    if (bArmed && CurrentBow && LeftHand)
    {
        LeftHand->GrabObject(CurrentBow);
    }
}
```

---

## 四、弓箭交互流程总结

### 4.1 VR 模式

```
1. 玩家一只手抓弓身
   → LeftHand->GrabObject(Bow)
   → Bow::OnGrabbed → bBodyHeld=true, GrabType=WeaponSnap

2. 玩家另一只手抓箭，手移入弓弦区域
   → Bow::OnStringCollisionBeginOverlap 检测到手
   → 手持有箭：RightHand->ReleaseObject() + Bow->NockArrow(Arrow)
   → 手抓弓弦：RightHand->GrabObject(Bow)
   → Bow::OnGrabbed → bStringHeld=true, GrabType=Custom

3. 玩家拉动手
   → Bow::Tick 更新弓弦位置（跟随 StringHoldingHand）

4. 玩家松开 Grip
   → RightHand->ReleaseObject()
   → Bow::OnReleased → ReleaseString() → FireArrow()
```

### 4.2 PC 模式

```
1. 玩家进入弓箭模式
   → SetBowArmed(true)
   → 生成弓，LeftHand->GrabObject(Bow)

2. 玩家按下瞄准键（LT）
   → StartAiming()
   → 生成箭并搭载（SpawnAndNockArrow）
   → 左手 Interp 到瞄准位置

3. 玩家按下拉弓键（RT）
   → StartDrawBow()
   → 生成箭，RightHand->GrabObject(Arrow)
   → 瞬移右手到弓弦位置 → 触发 overlap → 自动搭箭 + 抓弓弦
   → Interp 右手到拉弓位置

4. Tick
   → 弓弦跟随 RightHand 位置

5. 玩家松开 RT
   → ReleaseBowString()
   → RightHand->ReleaseObject()
   → Bow::OnReleased → 发射箭
```

---

## 五、需要蓝图配置的参数

| 参数 | 所属类 | 说明 |
|-----|-------|------|
| `WeaponSnapStrength` | PlayerGrabHand | WeaponSnap PhysicsControl 强度 |
| `WeaponSnapDamping` | PlayerGrabHand | WeaponSnap PhysicsControl 阻尼 |
| `HandCollision->SphereRadius` | PlayerGrabHand | 手部碰撞半径 |
| `StringCollisionTransform` | BasePCPlayer | PC模式弓弦位置（相对于摄像机） |
| `DrawingRightHandTransform` | BasePCPlayer | PC模式拉弓位置（相对于摄像机） |

---

## 六、实现顺序

1. **阶段一：抓取系统基础重构**
   - [ ] GrabTypes.h - 移除 Snap
   - [ ] IGrabbable.h - 移除 GetSnapOffset
   - [ ] GrabbeeObject - 移除 SnapOffset 相关
   - [ ] PlayerGrabHand - 添加 HandCollision
   - [ ] PlayerGrabHand - 移除 ValidateGrab, ReleaseAttachOnly, GrabSnap
   - [ ] PlayerGrabHand - 修改 GrabWeaponSnap 使用 PhysicsControl
   - [ ] PlayerGrabHand - 修改 GrabObject 添加检查，移除 Snap 分支
   - [ ] PlayerGrabHand - 修改 ReleaseObject 移除 ReleaseAttach

2. **阶段二：敌人双手抓取**
   - [ ] BaseEnemy - 添加 ControllingHands
   - [ ] BaseEnemy - 完善 OnGrabbed/OnReleased

3. **阶段三：弓箭系统重构**
   - [ ] Bow - 移除 SetHandInStringArea, UpdateStringFromHandPosition, SetPullAmount
   - [ ] Bow - StartPullingString 改 private
   - [ ] Bow - 修改 OnStringCollisionBeginOverlap 实现自动逻辑
   - [ ] Bow - 添加 GetHandFromCollision 辅助函数

4. **阶段四：玩家控制重构**
   - [ ] BasePCPlayer - 移除 StopDrawBow
   - [ ] BasePCPlayer - 修改 StartDrawBow 新流程
   - [ ] BasePCPlayer - 修改 ReleaseBowString
   - [ ] BaseVRPlayer - 修改 SetBowArmed 使用 ReleaseObject

5. **阶段五：编译验证和文档更新**
   - [ ] 编译通过
   - [ ] 更新 claude.md

---

## 七、风险和注意事项

1. **PhysicsControl 参数调整**：WeaponSnap 改用 PhysicsControl 后，需要调整强度/阻尼参数避免"软"的手感
2. **弓弦 Overlap 检测**：确保 HandCollision 和 StringCollision 的 collision channel 正确配置
3. **释放顺序**：SetBowArmed(false) 必须在 DestroyBow 之前完成所有 ReleaseObject 调用
4. **箭的状态管理**：确保自动搭箭时箭的状态正确转换（Idle → Nocked）

---

**最后更新**：2025-12-31
