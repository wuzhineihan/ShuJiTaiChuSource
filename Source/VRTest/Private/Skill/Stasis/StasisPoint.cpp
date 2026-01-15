// Skill/Stasis/StasisPoint.cpp

#include "Skill/Stasis/StasisPoint.h"

#include "Game/GameSettings.h"
#include "Grabber/PlayerGrabHand.h"
#include "Skill/SkillAsset.h"
#include "Skill/Stasis/IStasisable.h"

AStasisPoint::AStasisPoint()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create Sphere component
    Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
    Sphere->SetCollisionProfileName("OverlapAllDynamic");
    RootComponent = Sphere;

    // Create Niagara component as child of Sphere
    Niagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Niagara"));
    Niagara->SetupAttachment(Sphere);
    
    const UGameSettings* Settings = UGameSettings::Get();
    if (USkillAsset* SkillAsset = Settings ? Settings->GetSkillAsset() : nullptr)
    {
        Niagara->SetAsset(SkillAsset->StasisPointEffect);
    }

    // Initialize variables
    Target = nullptr;
    TimeToStasis = 0.0f;

    // Initialize physics parameters
    SpringStiffness = 1000.0f;
    Damping = 100.0f;
    SpringForceMin = 0.0f;
}

void AStasisPoint::BeginPlay()
{
    Super::BeginPlay();

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

void AStasisPoint::Fire(FVector InitVelocity, USceneComponent* TrackTarget)
{
    // Switch to track mode
    EnterTrackMode();

    // Set initial velocity
    SetCurrentVelocity(InitVelocity);

    // Set target
    Target = TrackTarget;

    // If no valid target, set target location far away in velocity direction
    if (!Target || !Target->IsValidLowLevel())
    {
        FVector CurrentLocation = GetActorLocation();
        FVector NormalizedVelocity = InitVelocity.GetSafeNormal();
        FVector FarLocation = CurrentLocation + (NormalizedVelocity * 1000000.0f);
        SetTargetLocation(FarLocation);
    }
}

void AStasisPoint::EnterFollowingMode()
{
    // Set physics parameters for following mode
    SpringStiffness = 1000.0f;
    Damping = 100.0f;
    SpringForceMin = 0.0f;

    // Apply to parent class if it has these properties
    // Note: You'll need to implement SetSpringStiffness, SetDamping, 
    // and SetSpringForceMin in AFakePhysicsHandleActor or call appropriate methods
}

void AStasisPoint::EnterTrackMode()
{
    // Set physics parameters for track mode
    SpringStiffness = 20.0f;
    Damping = 10.0f;
    SpringForceMin = 1000.0f;

    // Apply to parent class if it has these properties
}

void AStasisPoint::OnGrabbed_Implementation(UPlayerGrabHand* Hand)
{
    Target = Hand;
}

void AStasisPoint::OnReleased_Implementation(UPlayerGrabHand* Hand)
{
    // 设置 Target 为 self，停止跟随手部
    Target = nullptr;
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
    return true;
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
    if (!OtherActor)
    {
        return;
    }

    // Check if the other actor implements IStasisable
    if (OtherActor->GetClass()->ImplementsInterface(UStasisable::StaticClass()))
    {
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
