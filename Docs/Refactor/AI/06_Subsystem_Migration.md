# Subsystem 迁移方案

## MasterActor 现状问题

### 根本问题："越级指挥"

```
当前架构的权力流：
MasterActor
    ↓ (直接修改)
Controller 的 BIsCombat, CurrentTargetEnemy
    ↑ (回调通知)
Pawn 和 Components
```

**问题**：
- MasterActor 直接修改 Controller 内部状态 → 破坏封装
- 多条路径更新同一状态 → 不一致的数据
- 全场景搜索 `GetAllActorsOfClass()` → 性能问题

---

## Subsystem 方案

### 为什么用 WorldSubsystem？

| 特性 | Actor | WorldSubsystem |
|------|-------|-----------------|
| **生命周期** | 手动管理 Spawn/Destroy | 自动跟随 World |
| **唯一性** | 需要手动检查 | UE 保证唯一实例 |
| **初始化时机** | BeginPlay（不可靠） | Initialize（可靠） |
| **清理** | 手动 Destroy | Deinitialize（自动） |
| **全局访问** | 需要 GetWorld → GetActor | GetWorld → GetSubsystem |
| **跨 Map 持久化** | 可选配置 | 默认不持久化 |

### 新架构图

```
WorldSubsystem: SquadManagerSubsystem
    ↓
    Maintains: Map<ControllerId, ISquadMember>
    ↓
    Events:
    - OnEnemySpotted(Location, Enemy)
    - OnAllyNeedsSupport(Location)
    - OnSuspiciousLocation(Location)
    ↓
    Controllers 注册接口实现
    ↓
    ISquadMember interface:
        void ReceiveSquadOrder(EOrderType, FVector)
        void ReportStatus()
```

---

## 实现步骤

### 步骤 1：定义 ISquadMember 接口

```cpp
// C++ Class: ISquadMember
UINTERFACE(MinimalAPI, Blueprintable)
class USquadMember : public UInterface
{
    GENERATED_BODY()
};

class ISquadMember
{
    GENERATED_BODY()

public:
    // 控制器加入/离开小队
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void JoinSquad(class ASquadManagerSubsystem* SquadManager);
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void LeaveSquad();
    
    // 接收小队命令（替代 MasterActor 直接改状态）
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void ReceiveSquadNotification(
        ESquadNotificationType NotificationType,
        FVector Location,
        AActor* TargetEnemy
    );
    
    // 查询状态（Subsystem 用来做决策）
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    FVector GetCurrentLocation() const;
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    bool IsInCombat() const;
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    bool CanProvideCover() const;
};

// 通知类型定义
UENUM(BlueprintType)
enum class ESquadNotificationType : uint8
{
    EnemySpotted,
    NeedSupport,
    SuspiciousLocation,
    AllyCasualty,
    RetreatOrder,
    FormUp
};
```

### 步骤 2：创建 SquadManagerSubsystem

```cpp
// C++ Class: SquadManagerSubsystem
UCLASS()
class VRTEST_API ASquadManagerSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // 注册/注销小队成员
    UFUNCTION(BlueprintCallable)
    void RegisterSquadMember(AActor* Controller, int32 TeamId = 0);
    
    UFUNCTION(BlueprintCallable)
    void UnregisterSquadMember(AActor* Controller, int32 TeamId = 0);
    
    // 报告敌人（替代 MasterActor.FoundEnemy()）
    UFUNCTION(BlueprintCallable)
    void OnEnemySpotted(AActor* Spotter, FVector Location, AActor* Enemy);
    
    // 请求支援
    UFUNCTION(BlueprintCallable)
    void OnAllyRequestSupport(AActor* Requester, FVector Location);
    
    // 失去敌人
    UFUNCTION(BlueprintCallable)
    void OnEnemyLost(AActor* Reporter, AActor* Enemy);
    
    // 查询同队成员
    UFUNCTION(BlueprintCallable)
    TArray<AActor*> GetNearbyAllies(FVector Location, float Radius, int32 TeamId);

private:
    struct FSquadMember
    {
        AActor* Controller;
        ISquadMember* Interface;
        int32 TeamId;
        FVector LastKnownLocation;
        bool bInCombat;
    };
    
    // 按 TeamId 分组
    UPROPERTY()
    TMap<int32, TArray<FSquadMember>> SquadsByTeam;
    
    // 快速查找
    UPROPERTY()
    TMap<AActor*, FSquadMember*> ControllerToMember;
};
```

### 步骤 3：Subsystem 实现

```cpp
void ASquadManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // 在 Editor 中跳过
    if (GetWorld()->IsEditorWorld())
    {
        return;
    }
    
    // 初始化完毕
    UE_LOG(LogSquad, Warning, TEXT("SquadManagerSubsystem Initialized"));
}

void ASquadManagerSubsystem::Deinitialize()
{
    SquadsByTeam.Empty();
    ControllerToMember.Empty();
    
    Super::Deinitialize();
}

void ASquadManagerSubsystem::RegisterSquadMember(AActor* Controller, int32 TeamId)
{
    if (!Controller)
        return;
    
    if (!Controller->Implements<USquadMember>())
    {
        UE_LOG(LogSquad, Error, TEXT("Actor %s doesn't implement ISquadMember"), *Controller->GetName());
        return;
    }
    
    FSquadMember NewMember;
    NewMember.Controller = Controller;
    NewMember.Interface = Cast<ISquadMember>(Controller);
    NewMember.TeamId = TeamId;
    NewMember.LastKnownLocation = Controller->GetActorLocation();
    NewMember.bInCombat = false;
    
    if (!SquadsByTeam.Contains(TeamId))
    {
        SquadsByTeam.Add(TeamId, TArray<FSquadMember>());
    }
    
    SquadsByTeam[TeamId].Add(NewMember);
    ControllerToMember.Add(Controller, &SquadsByTeam[TeamId].Last());
    
    // 通知成员已加入
    ISquadMember::Execute_JoinSquad(Controller, this);
    
    UE_LOG(LogSquad, Warning, TEXT("Registered %s to Team %d"), *Controller->GetName(), TeamId);
}

void ASquadManagerSubsystem::UnregisterSquadMember(AActor* Controller, int32 TeamId)
{
    if (!Controller)
        return;
    
    if (ControllerToMember.Contains(Controller))
    {
        ISquadMember::Execute_LeaveSquad(Controller);
        ControllerToMember.Remove(Controller);
    }
    
    if (SquadsByTeam.Contains(TeamId))
    {
        SquadsByTeam[TeamId].RemoveAll([Controller](const FSquadMember& Member)
        {
            return Member.Controller == Controller;
        });
    }
}

void ASquadManagerSubsystem::OnEnemySpotted(AActor* Spotter, FVector Location, AActor* Enemy)
{
    if (!Spotter || !ControllerToMember.Contains(Spotter))
        return;
    
    FSquadMember* SpotterMember = ControllerToMember[Spotter];
    int32 TeamId = SpotterMember->TeamId;
    
    // 获取同队所有成员
    TArray<FSquadMember>* Squad = SquadsByTeam.Find(TeamId);
    if (!Squad)
        return;
    
    // 广播敌人情报给所有队友
    for (FSquadMember& Member : *Squad)
    {
        if (Member.Controller != Spotter)  // 不通知自己
        {
            ISquadMember::Execute_ReceiveSquadNotification(
                Member.Controller,
                ESquadNotificationType::EnemySpotted,
                Location,
                Enemy
            );
        }
    }
    
    UE_LOG(LogSquad, Warning, TEXT("Enemy spotted at %s, notified %d allies"), *Location.ToString(), Squad->Num() - 1);
}

void ASquadManagerSubsystem::OnAllyRequestSupport(AActor* Requester, FVector Location)
{
    if (!Requester || !ControllerToMember.Contains(Requester))
        return;
    
    FSquadMember* RequesterMember = ControllerToMember[Requester];
    int32 TeamId = RequesterMember->TeamId;
    
    TArray<FSquadMember>* Squad = SquadsByTeam.Find(TeamId);
    if (!Squad)
        return;
    
    // 找离请求者最近的盟友
    AActor* BestSupport = nullptr;
    float BestDistance = FLT_MAX;
    
    for (FSquadMember& Member : *Squad)
    {
        if (Member.Controller != Requester && Member.bInCombat)  // 只有在战斗中的才能支援
        {
            float Distance = FVector::Dist(Member.LastKnownLocation, Location);
            if (Distance < BestDistance)
            {
                BestDistance = Distance;
                BestSupport = Member.Controller;
            }
        }
    }
    
    if (BestSupport)
    {
        ISquadMember::Execute_ReceiveSquadNotification(
            BestSupport,
            ESquadNotificationType::NeedSupport,
            Location,
            Requester
        );
    }
}

TArray<AActor*> ASquadManagerSubsystem::GetNearbyAllies(FVector Location, float Radius, int32 TeamId)
{
    TArray<AActor*> Result;
    
    TArray<FSquadMember>* Squad = SquadsByTeam.Find(TeamId);
    if (!Squad)
        return Result;
    
    for (FSquadMember& Member : *Squad)
    {
        float Distance = FVector::Dist(Member.LastKnownLocation, Location);
        if (Distance <= Radius)
        {
            Result.Add(Member.Controller);
        }
    }
    
    return Result;
}
```

### 步骤 4：Controller 改造

#### 旧代码（与 MasterActor 紧耦合）

```cpp
void AGoap_EnemyAI_Controller::OnMasterActorSpottedEnemy(AActor* Enemy, FVector Location)
{
    // ❌ 直接改状态
    BIsCombat = true;
    CurrentTargetEnemy = Enemy;
    CombatWarningProcess = 1.0f;
    
    // ❌ 直接调用 UI
    if (UIComponent)
    {
        UIComponent->EnterState(EEnemyState::Fight);
    }
}
```

#### 新代码（实现 ISquadMember 接口）

```cpp
// 声明实现接口
class AGoap_EnemyAI_Controller : public AAIController, public ISquadMember
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    
    // ISquadMember 实现
    virtual void JoinSquad_Implementation(ASquadManagerSubsystem* SquadManager) override;
    virtual void LeaveSquad_Implementation() override;
    virtual void ReceiveSquadNotification_Implementation(
        ESquadNotificationType NotificationType,
        FVector Location,
        AActor* TargetEnemy
    ) override;

private:
    TWeakObjectPtr<ASquadManagerSubsystem> CachedSquadManager;
};

void AGoap_EnemyAI_Controller::BeginPlay()
{
    Super::BeginPlay();
    
    // 注册到 Subsystem
    if (ASquadManagerSubsystem* SquadManager = GetWorld()->GetSubsystem<ASquadManagerSubsystem>())
    {
        SquadManager->RegisterSquadMember(this, TeamId);  // TeamId 从配置或 Pawn 获取
        CachedSquadManager = SquadManager;
    }
}

void AGoap_EnemyAI_Controller::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (CachedSquadManager.IsValid())
    {
        CachedSquadManager->UnregisterSquadMember(this, TeamId);
    }
    
    Super::EndPlay(EndPlayReason);
}

void AGoap_EnemyAI_Controller::JoinSquad_Implementation(ASquadManagerSubsystem* SquadManager)
{
    CachedSquadManager = SquadManager;
    UE_LOG(LogSquad, Warning, TEXT("%s joined squad"), *GetName());
}

void AGoap_EnemyAI_Controller::LeaveSquad_Implementation()
{
    CachedSquadManager = nullptr;
}

void AGoap_EnemyAI_Controller::ReceiveSquadNotification_Implementation(
    ESquadNotificationType NotificationType,
    FVector Location,
    AActor* TargetEnemy
)
{
    // ✅ 不再直接改状态，而是转发给 AlertnessComponent
    switch (NotificationType)
    {
        case ESquadNotificationType::EnemySpotted:
            if (AlertnessComponent)
            {
                // AlertnessComponent 决定是否进入战斗
                AlertnessComponent->ReceiveSquadIntel(Location, TargetEnemy);
            }
            break;
            
        case ESquadNotificationType::NeedSupport:
            // 转发给 CombatComponent（之后会新建）
            RequestSupportAlly(Location, TargetEnemy);
            break;
            
        default:
            break;
    }
}

float AGoap_EnemyAI_Controller::GetCurrentLocation() const
{
    return GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
}

bool AGoap_EnemyAI_Controller::IsInCombat() const
{
    if (APawn* ControlledPawn = GetPawn())
    {
        if (ControlledPawn->Implements<IEnemyPawn>())
        {
            // 查询 AlertnessComponent 的状态
            return IEnemyPawn::Execute_IsInCombat(ControlledPawn);
        }
    }
    return false;
}
```

### 步骤 5：删除 MasterActor 直接操作

#### 旧代码（MasterActor 直接改 Controller 状态）

```cpp
// ❌ MasterActor.cpp
void AMasterActor::FoundEnemy(AActor* EnemyRef, FVector Location, AGoap_EnemyAI_Controller* ControllerRef)
{
    if (!ControllerRef) return;
    
    // 直接改状态 → 破坏封装
    ControllerRef->BIsCombat = true;
    ControllerRef->CurrentTargetEnemy = EnemyRef;
    ControllerRef->CombatWarningProcess = 1.0f;
    
    // 直接调用 UI → 暴露实现
    ControllerRef->UIComponent->EnterState(EEnemyState::Fight);
}
```

#### 新代码（通过 Subsystem）

```cpp
// ✅ MasterActor.cpp（或者完全删除这个类）
void AMasterActor::FoundEnemy(FVector Location, AActor* EnemyRef)
{
    if (ASquadManagerSubsystem* SquadManager = GetWorld()->GetSubsystem<ASquadManagerSubsystem>())
    {
        // 不传 Controller！只传敌人信息
        SquadManager->OnEnemySpotted(this, Location, EnemyRef);
        
        // 或者由发现敌人的 Controller 自己通知
        // (见 OnTargetPerceptionUpdated 的改造)
    }
}
```

或者完全由 **Controller 的感知系统** 自己通知 Subsystem：

```cpp
// ✅ Controller.cpp
void AGoap_EnemyAI_Controller::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (Stimulus.WasSuccessfullySensed())
    {
        // 自己发现了敌人
        if (ASquadManagerSubsystem* SquadManager = GetWorld()->GetSubsystem<ASquadManagerSubsystem>())
        {
            // 通知小队成员
            SquadManager->OnEnemySpotted(this, Actor->GetActorLocation(), Actor);
        }
    }
}
```

---

## 迁移清单

### Phase 1: 接口与基础设施

- [ ] 创建 `ISquadMember` 接口
- [ ] 创建 `ASquadManagerSubsystem` 类
- [ ] 编写 Subsystem 完整实现
- [ ] 测试 Subsystem 的注册/注销/广播

### Phase 2: Controller 改造

- [ ] Controller 实现 `ISquadMember`
- [ ] 在 BeginPlay 注册，在 EndPlay 注销
- [ ] 实现 `ReceiveSquadNotification` 事件处理
- [ ] 移除对 MasterActor 的硬编码依赖

### Phase 3: MasterActor 删除（可选）

- [ ] 如果 MasterActor 还有其他用途，改造为 Subsystem 的客户端
- [ ] 或者完全删除，由各 Controller 自己通知 Subsystem

### Phase 4: 事件驱动补全

- [ ] AlertnessComponent 添加 `OnAlertStateChanged` 委托
- [ ] Pawn 监听该委托并更新 UI
- [ ] 验证事件链是否正确

---

## 性能对比

| 操作 | 旧方案（MasterActor） | 新方案（Subsystem） |
|------|----------------------|-------------------|
| 发现敌人通知 | 逐个遍历，直接改状态 | 广播事件，接收者监听 |
| 查询同队成员 | `GetAllActorsOfClass()` 全场景搜索 | Map 快速查找 O(1) |
| 请求支援 | 调用 MasterActor 再遍历 | 直接查询 Subsystem |
| 内存占用 | 单个 Actor 实例 | 单个 Subsystem 实例 |
| 扩展性 | 差（MasterActor 需改） | 好（只改 Controller） |

---

## 调试技巧

### 启用日志

```cpp
// DefaultEngine.ini
[Core.Log]
LogSquad=Warning

// 代码中使用
UE_LOG(LogSquad, Warning, TEXT("Message here"));
```

### 可视化调试

```cpp
// 在 Subsystem 中绘制小队成员位置
#if WITH_EDITOR
void ASquadManagerSubsystem::DrawDebug() const
{
    for (const auto& TeamPair : SquadsByTeam)
    {
        for (const FSquadMember& Member : TeamPair.Value)
        {
            if (Member.Controller)
            {
                DrawDebugSphere(GetWorld(), Member.LastKnownLocation, 50, 8, FColor::Green, false, 0.1f);
                DrawDebugString(GetWorld(), Member.LastKnownLocation, *Member.Controller->GetName(), nullptr, FColor::White, 0.1f);
            }
        }
    }
}
#endif
```

### Blueprint 调试

在 Blueprint 中添加 Print 节点监听 ReceiveSquadNotification：

```blueprint
Event ReceiveSquadNotification
    Print (NotificationType, Location, TargetEnemy)
```

---

## 常见问题

### Q: 如果 Controller 在 Subsystem 初始化前 BeginPlay 会怎样？

**A**: UE 保证 Subsystem 在 World 创建时初始化，早于任何 Actor 的 BeginPlay。但为了安全：

```cpp
void AGoap_EnemyAI_Controller::BeginPlay()
{
    Super::BeginPlay();
    
    ASquadManagerSubsystem* SquadManager = GetWorld()->GetSubsystem<ASquadManagerSubsystem>();
    if (!SquadManager)
    {
        UE_LOG(LogSquad, Error, TEXT("SquadManagerSubsystem not found!"));
        return;
    }
    
    SquadManager->RegisterSquadMember(this, TeamId);
}
```

### Q: 如何在 PIE 和 Shipping 中表现不同？

**A**:

```cpp
void ASquadManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    if (GetWorld()->IsEditorWorld())
    {
        return;  // 在 Editor 中不初始化
    }
    
    #if WITH_EDITOR
    bDebugDraw = true;
    #else
    bDebugDraw = false;
    #endif
}
```

### Q: 多个 Team 怎么处理？

**A**: Subsystem 已支持多队伍（`TeamId`）：

```cpp
// Team 0: 敌人
SquadManager->RegisterSquadMember(EnemyController, 0);

// Team 1: 友军
SquadManager->RegisterSquadMember(AllyController, 1);

// 只向 Team 0 广播
SquadManager->OnEnemySpotted(Spotter, Location, Enemy);
```

