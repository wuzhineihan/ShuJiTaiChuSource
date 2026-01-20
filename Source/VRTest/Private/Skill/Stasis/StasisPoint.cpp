// Skill/Stasis/StasisPoint.cpp

#include "Skill/Stasis/StasisPoint.h"

#include "Game/GameSettings.h"
#include "Grabber/PlayerGrabHand.h"
#include "Skill/SkillAsset.h"
#include "Skill/Stasis/IStasisable.h"
#include "Game/CollisionConfig.h"
#include "Tools/GameUtils.h"
#include "Engine/World.h"
#include "TimerManager.h"

AStasisPoint::AStasisPoint()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create Sphere component
    Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
    Sphere->SetCollisionProfileName(CP_NO_COLLISION);
    Sphere->SetSphereRadius(32);
    RootComponent = Sphere;

    // Create Niagara component as child of Sphere
    Niagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Niagara"));
    Niagara->SetupAttachment(Sphere);

    // Initialize variables
    Target = nullptr;
    TimeToStasis = 0.0f;

    // 定身目标检测默认 ObjectTypes：WorldDynamic + Pawn
    TargetingObjectTypes = {
        UEngineTypes::ConvertToObjectType(ECC_WorldDynamic),
        UEngineTypes::ConvertToObjectType(ECC_PhysicsBody)
    };

    // Initialize physics parameters
    SpringStiffness = 1000.0f;
    Damping = 100.0f;
    SpringForceMin = 0.0f;
}

void AStasisPoint::BeginPlay()
{
    Super::BeginPlay();
    
    const UGameSettings* Settings = UGameSettings::Get();
    if (USkillAsset* SkillAsset = Settings ? Settings->GetSkillAsset() : nullptr)
    {
        Niagara->SetAsset(SkillAsset->StasisPointEffect);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SkillAsset is nullptr"));
    }

    // Start physics simulation
    StartSimulate();

    // Enter following mode by default
    EnterFollowingMode();

    // Bind overlap event
    if (Sphere)
    {
        Sphere->OnComponentBeginOverlap.AddDynamic(this, &AStasisPoint::OnSphereBeginOverlap);
    }
}

void AStasisPoint::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update target location if valid
    if (Target && Target->IsValidLowLevel())
    {
        SetTargetLocation(Target->GetComponentLocation());
    }
}

void AStasisPoint::FireWithTargetActor(FVector InitVelocity, AActor* TrackTargetActor)
{
    // 发射前：恢复自身碰撞（否则无法触发 Overlap），但仍应忽略玩家手持物体
    RestorePostFireCollisionRules();

    // Switch to track mode
    EnterTrackMode();

    // Set initial velocity
    SetCurrentVelocity(InitVelocity);

    // Set target component chosen from actor
    Target = ChooseTrackComponent(TrackTargetActor);

    // If no valid target, keep old fallback behavior (far away)
    if (!Target || !Target->IsValidLowLevel())
    {
        const FVector NormalizedVelocity = InitVelocity.GetSafeNormal();
        const FVector FarLocation = GetActorLocation() + (NormalizedVelocity * 1000000.0f);
        SetTargetLocation(FarLocation);
    }
}

AActor* AStasisPoint::FindStasisTarget(
    UObject* WorldContextObject,
    const FVector& Origin,
    const FVector& AimDirection,
    float DetectionRadius,
    float DetectionAngleDegrees,
    const TArray<AActor*>& IgnoreActors) const
{
    // 复用现有 GameUtils：SphereOverlap + 角度排序
    const TArray<FActorWithAngle> ActorsInCone = UGameUtils::FindActorsInCone(
        WorldContextObject,
        Origin,
        AimDirection,
        DetectionRadius,
        DetectionAngleDegrees,
        TargetingObjectTypes,
        IgnoreActors
    );

    for (const FActorWithAngle& Item : ActorsInCone)
    {
        AActor* Actor = Item.Actor;
        if (!Actor)
        {
            continue;
        }

        if (!Actor->GetClass()->ImplementsInterface(UStasisable::StaticClass()))
        {
            continue;
        }

        if (!IStasisable::Execute_CanEnterStasis(Actor))
        {
            continue;
        }

        return Actor;
    }

    return nullptr;
}

void AStasisPoint::StartNoTargetFlight(const FVector& Origin, const FVector& AimDirection, float MaxNoTargetDistance, float NoTargetLifeSeconds)
{
    const FVector Dir = AimDirection.GetSafeNormal();
    const FVector FallbackDir = Dir.IsNearlyZero() ? GetActorForwardVector() : Dir;

    SetTargetLocation(Origin + (FallbackDir * MaxNoTargetDistance));

    // 超时自毁：避免无限飞行常驻
    if (NoTargetLifeSeconds > 0.0f)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(NoTargetDestroyTimerHandle);
            World->GetTimerManager().SetTimer(
                NoTargetDestroyTimerHandle,
                this,
                &AActor::K2_DestroyActor,
                NoTargetLifeSeconds,
                false
            );
        }
    }
}

void AStasisPoint::Fire(
    UObject* WorldContextObject,
    const FVector& Origin,
    const FVector& AimDirection,
    const FVector& InitVelocity,
    float DetectionRadius,
    float DetectionAngleDegrees,
    const TArray<AActor*>& IgnoreActors,
    float MaxNoTargetDistance,
    float NoTargetLifeSeconds)
{
    // 发射前：打开碰撞（保证能触发 Overlap）并保持忽略玩家相关 Actor 的规则
    RestorePostFireCollisionRules();

    // 进入追踪模式
    EnterTrackMode();

    // 设置初速度
    SetCurrentVelocity(InitVelocity);

    // Fire 时不再依赖 Held 逻辑；若有旧的超时任务，清掉
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(NoTargetDestroyTimerHandle);
    }

    // 由定身球内部选目标
    AActor* TargetActor = FindStasisTarget(
        WorldContextObject,
        Origin,
        AimDirection,
        DetectionRadius,
        DetectionAngleDegrees,
        IgnoreActors
    );

    Target = ChooseTrackComponent(TargetActor);

    if (!Target || !Target->IsValidLowLevel())
    {
        // 没目标：向足够远处直飞，并在一段时间后自毁
        StartNoTargetFlight(Origin, AimDirection, MaxNoTargetDistance, NoTargetLifeSeconds);
    }
}

void AStasisPoint::OnGrabbed_Implementation(UPlayerGrabHand* Hand)
{
    HoldingHand = Hand;
    Target = Hand;
    bGrabbed = true;

    ApplyHeldCollisionRules();
}

void AStasisPoint::OnReleased_Implementation(UPlayerGrabHand* Hand)
{
    // 注意：这里的 Released ��一定代表“发射”，PC 投掷/VR 发射会在 ReleaseObject 后再调用 Fire
    // 为保证发射阶段可正常 Overlap，这里仅清理跟随目标与握持状态，不在这里强制恢复碰撞。
    Target = nullptr;
    bGrabbed = false;

    // 如果是普通丢弃导致的 Release（非 Fire），可能需要恢复碰撞；
    // 但当前定身球预期只通过技能系统发射，这里保守处理：若未进入发射则依然保持禁用，避免马上撞手。

    HoldingHand = nullptr;
}

// ==================== IGrabbable 接口实现 ====================

EGrabType AStasisPoint::GetGrabType_Implementation() const
{
    // Custom 类型：不使用 PhysicsHandle，StasisPoint 有自己的物理逻辑（FakePhysics）
    return EGrabType::Custom;
}

UPrimitiveComponent* AStasisPoint::GetGrabPrimitive_Implementation() const
{
    return Sphere;
}

bool AStasisPoint::CanBeGrabbedBy_Implementation(const UPlayerGrabHand* Hand) const
{
    // StasisPoint 只能被技能系统通过 GrabObject 直接抓取，不能通过 TryGrab
    // 但由于技能系统会直接调用 GrabObject，这里返回 true 即可
    return !bGrabbed;
}

bool AStasisPoint::CanBeGrabbedByGravityGlove_Implementation() const
{
    // StasisPoint 不支持重力手套
    return false;
}

bool AStasisPoint::SupportsDualHandGrab_Implementation() const
{
    return false;
}

void AStasisPoint::OnGrabSelected_Implementation()
{
    // StasisPoint 不支持重力手套选中
}

void AStasisPoint::OnGrabDeselected_Implementation()
{
    // StasisPoint 不支持重力手套选中
}

void AStasisPoint::OnSphereBeginOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
                    TEXT("Enter Stasis - Overlapped Actor: ") + OtherActor->GetName());
    // Check if the other actor implements IStasisable
    if (OtherActor->GetClass()->ImplementsInterface(UStasisable::StaticClass()))
    {
        if (!Execute_CanEnterStasis(OtherActor))
            return;
        
        // Debug message
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
                TEXT("Enter Stasis - Target is BPI_Stasisable"));
        }
        
        // Call EnterStasis on the interface
        Execute_EnterStasis(OtherActor, TimeToStasis);

        // Destroy this actor
        Destroy();
    }
}

void AStasisPoint::ApplyHeldCollisionRules()
{
    // 1) 握在手里时直接关掉自身碰撞，避免与另一只手抓取物体互撞
    if (Sphere)
    {
        Sphere->SetCollisionProfileName(CP_NO_COLLISION);
    }

    // 2) 同时设置忽略：避免即便打开碰撞后（发射/其他原因）也与玩家手持物体互撞
    //    这里使用 IgnoreActorWhenMoving，对 sweep / MoveComponent 有效；对物理碰撞也尽量通过 MoveIgnore 列表覆盖。
    if (!HoldingHand)
    {
        return;
    }

    auto ApplyIgnoreForHand = [this](UPlayerGrabHand* Hand)
    {
        if (!Hand)
        {
            return;
        }

        // 忽略手本身的 Owner（通常是玩家 Pawn）
        if (AActor* HandOwner = Hand->GetOwner())
        {
            if (Sphere)
            {
                Sphere->IgnoreActorWhenMoving(HandOwner, true);
            }
        }

        // 忽略手里拿着的物体
        if (AActor* Held = Hand->HeldActor)
        {
            if (Sphere)
            {
                Sphere->IgnoreActorWhenMoving(Held, true);
            }
        }
    };

    ApplyIgnoreForHand(HoldingHand);
    ApplyIgnoreForHand(HoldingHand->OtherHand);
}

void AStasisPoint::RestorePostFireCollisionRules()
{
    // 发射后需要重新打开碰撞，才能触发 Sphere overlap 来进入定身
    if (Sphere)
    {
        Sphere->SetCollisionProfileName(CP_STASIS_POINT_FIRED);
        Sphere->SetGenerateOverlapEvents(true);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
                    TEXT("Stasis Point Collision Restored"));
    }

    // 继续保持对玩家相关 Actor 的忽略，防止刚发射就撞到玩家双手持有物
    if (HoldingHand)
    {
        auto ApplyIgnoreForHand = [this](UPlayerGrabHand* Hand)
        {
            if (!Hand)
            {
                return;
            }

            if (AActor* HandOwner = Hand->GetOwner())
            {
                if (Sphere)
                {
                    Sphere->IgnoreActorWhenMoving(HandOwner, true);
                }
            }

            if (AActor* Held = Hand->HeldActor)
            {
                if (Sphere)
                {
                    Sphere->IgnoreActorWhenMoving(Held, true);
                }
            }
        };

        ApplyIgnoreForHand(HoldingHand);
        ApplyIgnoreForHand(HoldingHand->OtherHand);
    }
}

USceneComponent* AStasisPoint::ChooseTrackComponent(AActor* TargetActor)
{
    if (!TargetActor)
    {
        return nullptr;
    }

    // 默认追踪根组件
    if (USceneComponent* Root = TargetActor->GetRootComponent())
    {
        return Root;
    }

    // 兜底：取第一个 SceneComponent
    TArray<USceneComponent*> SceneComps;
    TargetActor->GetComponents<USceneComponent>(SceneComps);
    return SceneComps.Num() > 0 ? SceneComps[0] : nullptr;
}

void AStasisPoint::EnterFollowingMode()
{
    // following：更“紧”地跟随手
    SpringStiffness = 1000.0f;
    Damping = 100.0f;
    SpringForceMin = 0.0f;
}

void AStasisPoint::EnterTrackMode()
{
    // track：更“软”地追踪目标，让飞行更平滑
    SpringStiffness = 20.0f;
    Damping = 10.0f;
    SpringForceMin = 1000.0f;
}
