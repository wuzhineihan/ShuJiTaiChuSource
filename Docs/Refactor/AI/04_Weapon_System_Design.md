# 武器系统多态设计

## 当前问题

弓箭与近战攻击的流程差异较大：
- **弓箭**：有瞄准阶段，可以调整方向，需要等待最小瞄准时间或准星稳定
- **近战**：快速挥舞，基本无准备阶段

当前实现在 `BP_GoapEnemy.Attack` 接口中用 `Switch on EnemyType` 硬编码：
```blueprint
Switch on Enum Enemy_Type
    |-- Archer: ArcherComponent.Archer_Attack()
    |-- Normal: PlayAnimMontage(Attack_PrimaryA_Montage)
```

**问题**：
1. 无法统一两种攻击的调用方式
2. 新增攻击类型需要修改 Pawn
3. 无法灵活切换武器

---

## 统一接口设计

### 1. 分阶段攻击接口

定义统一的 `BPI_Attackable` 接口，所有攻击组件都实现它。

```cpp
Interface BPI_Attackable
{
    // 查询能否接受攻击请求
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    bool RequestAttack(AActor* Target);
    
    // 获取有效攻击距离
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    float GetDesiredRange();
    
    // 获取优先级（选择器用来挑选最佳攻击）
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    int32 GetPriority();
    
    // 进入预备状态（拉弦、抬手等）
    // 返回 Handle，用于后续 Commit/Cancel
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    FAttackHandle PrepareAttack(AActor* Target);
    
    // 是否已准备好出手（弓箭用于瞄准准备）
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    bool IsReadyToFire();
    
    // 真正出手
    // 返回完成/冷却时间
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    float CommitAttack(FAttackHandle Handle);
    
    // 中止攻击（目标丢失或需要切换）
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void CancelAttack(FAttackHandle Handle);
}
```

---

### 2. 弓箭组件实现

#### PrepareAttack
```cpp
PrepareAttack(Target):
    Handle = CreateAttackHandle()
    
    // 进入瞄准状态
    CurrentState = Aiming
    AimStartTime = CurrentTime
    PlayAnimMontage(Bow_Equip)
    
    // 设置最小瞄准时间
    MinAimTime = 0.5 seconds
    
    Return Handle
```

#### IsReadyToFire
```cpp
IsReadyToFire():
    If CurrentState != Aiming:
        Return false
    
    TimeSinceAim = CurrentTime - AimStartTime
    
    If TimeSinceAim < MinAimTime:
        Return false
    
    // 可选：检查准星稳定性（偏差 < 阈值）
    If ReticleDrift > MaxDrift:
        Return false
    
    Return true
```

#### CommitAttack
```cpp
CommitAttack(Handle):
    If !IsValidHandle(Handle):
        Return -1
    
    // 播放射箭动画
    DeltaTime = PlayAnimMontage(Bow_Shoot)
    
    // 生成/释放箭
    Arrow = SpawnArrow(ArrowSocket)
    
    // 计算弹道
    CalculateProjectileVelocity(Target, Arrow)
    
    // 启用物理
    Arrow.LaunchProjectile(Velocity)
    
    // 清空状态
    CurrentState = Idle
    
    // 设置冷却
    CooldownUntil = CurrentTime + CooldownDuration
    
    Return DeltaTime
```

#### CancelAttack
```cpp
CancelAttack(Handle):
    If !IsValidHandle(Handle):
        Return
    
    CurrentState = Idle
    StopAnimMontage(BowAnimMontage)
    
    If Arrow.IsValid() && !Arrow.IsLaunched():
        Arrow.Destroy()
```

---

### 3. 近战组件实现

#### PrepareAttack
```cpp
PrepareAttack(Target):
    Handle = CreateAttackHandle()
    
    // 快速准备（几乎瞬时）
    PlayAnimMontage(MeleePrep)  // 抬手，很短 0.1s
    
    Return Handle
```

#### IsReadyToFire
```cpp
IsReadyToFire():
    If CurrentState != Preparing:
        Return false
    
    // 抬手动画是否快结束？
    If MeleePrep.IsNearEnd():
        Return true
    
    Return false
```

#### CommitAttack
```cpp
CommitAttack(Handle):
    If !IsValidHandle(Handle):
        Return -1
    
    // 播放挥舞动画
    DeltaTime = PlayAnimMontage(MeleeAttack)
    
    // 设置伤害判定窗口
    // 在 0.3 - 0.5 秒之间检测碰撞
    ActivateDamageWindow(0.3, 0.5)
    
    // 清空状态
    CurrentState = Idle
    
    // 设置冷却
    CooldownUntil = CurrentTime + CooldownDuration
    
    Return DeltaTime
```

#### CancelAttack
```cpp
CancelAttack(Handle):
    StopAnimMontage(MeleeAttack)
    DeactivateDamageWindow()
    CurrentState = Idle
```

---

### 4. GetPriority 配置

**弓箭**：
```cpp
GetPriority():
    If Distance < 300:  // 太近
        Return 5
    Else If Distance > 1500:  // 太远
        Return 5
    Else:  // 正常范围
        Return 20
```

**近战**：
```cpp
GetPriority():
    If Distance < 300:  // 贴脸
        Return 25
    Else:  // 距离远
        Return 10
```

---

## 执行层集成

### 在 Controller 或 CombatComponent 中

#### 攻击选择器

```blueprint
SelectBestAttack(Target) -> BPI_Attackable:
    Distance = GetDistanceTo(Target)
    
    AllAttackComps = GetComponentsByInterface(BPI_Attackable)
    
    ValidAttacks = []
    For Each Comp in AllAttackComps:
        If Comp.RequestAttack(Target):
            If Distance <= Comp.GetDesiredRange():
                ValidAttacks.Add(Comp)
    
    If ValidAttacks.IsEmpty:
        Return None
    
    ValidAttacks.SortByDescending(Comp.GetPriority())
    Return ValidAttacks[0]
```

#### 攻击执行流

```blueprint
ExecuteAttack(Target):
    SelectedAttack = SelectBestAttack(Target)
    
    If !SelectedAttack:
        Return FAILED
    
    // 1. 准备阶段
    Handle = SelectedAttack.PrepareAttack(Target)
    
    // 2. 等待准备完成
    // 对于弓箭，可能要等待 0.5~2 秒
    // 对于近战，几乎瞬时
    WaitFor(SelectedAttack.IsReadyToFire())  // 可选等待
    
    // 3. 出手
    DeltaTime = SelectedAttack.CommitAttack(Handle)
    
    // 4. 等待攻击完成
    WaitFor(DeltaTime)
    
    // 5. 下一个动作
    Return SUCCESS
```

---

## GOAP 和任务执行器集成

### GOAP Action 示例

```cpp
Class GoapAction_Attack : public UGoapAction
{
    AActor* Target;
    
    OnEnter():
        SelectedAttack = Controller.SelectBestAttack(Target)
        If !SelectedAttack:
            FinishAction(FAILED)
            Return
        
        AttackHandle = SelectedAttack.PrepareAttack(Target)
        State = PREPARING
    
    OnTick():
        Switch State:
            PREPARING:
                If SelectedAttack.IsReadyToFire():
                    Duration = SelectedAttack.CommitAttack(AttackHandle)
                    State = EXECUTING
                    ElapsedTime = 0
            
            EXECUTING:
                ElapsedTime += DeltaTime
                If ElapsedTime >= Duration:
                    FinishAction(SUCCESS)
}
```

### 自定义任务执行器示例

```cpp
Class FAttackTask : public FGoapTask
{
    BPI_Attackable* SelectedWeapon;
    float RemainingTime;
    
    virtual void Start()
    {
        SelectedWeapon = Owner->SelectBestAttack(Target);
        SelectedWeapon->PrepareAttack(Target);
    }
    
    virtual void Update(float DeltaTime)
    {
        if (!SelectedWeapon->IsReadyToFire())
            return;  // 继续等待
        
        RemainingTime = SelectedWeapon->CommitAttack(Handle);
    }
    
    virtual bool IsFinished() const
    {
        RemainingTime -= DeltaTime;
        return RemainingTime <= 0;
    }
}
```

---

## 数据驱动配置

为不同武器在 DataTable 或 DataAsset 中配置参数：

```
DA_ArcherConfig:
  MinAimTime: 0.5
  MaxAimTime: 3.0
  MinRange: 500
  MaxRange: 1500
  Priority: 20
  CooldownDuration: 2.0
  HasAmmo: True
  
DA_MeleeConfig:
  MinRange: 0
  MaxRange: 300
  Priority: 25
  CooldownDuration: 1.5
  DamageWindow: [0.3, 0.5]
```

组件在初始化时读取配置，避免硬编码。

---

## 好处总结

| 好处 | 说明 |
|------|------|
| **统一接口** | 所有攻击都通过相同的调用链：Prepare → Ready → Commit |
| **灵活性** | 弓箭的瞄准、近战的快速挥舞都在组件内部处理 |
| **可扩展性** | 新增"法术攻击"、"远程投掷"等只需实现接口 |
| **数据驱动** | 参数都在 DataTable/DataAsset，无需改代码 |
| **可打断** | `CancelAttack()` 支持目标丢失时立即切换 |
| **选择器** | 自动根据距离和优先级选择最佳武器 |

---

## 实施建议

### Phase 1：建立接口
1. 定义 `BPI_Attackable` 接口
2. 让 ArcherComponent 和 MeleeComponent 都实现它

### Phase 2：迁移逻辑
1. 将弓箭逻辑分阶段实现（Prepare/Ready/Commit）
2. 将近战逻辑从 Pawn.Attack 迁移到 MeleeComponent
3. 移除 Pawn 的 `Switch on EnemyType` 硬编码

### Phase 3：集成执行层
1. 在 Controller 或 CombatComponent 中实现选择器
2. GOAP/TaskExecutor 通过接口调用攻击

### Phase 4：数据驱动
1. 为每种武器创建 DataAsset 配置
2. 组件从配置读取参数
