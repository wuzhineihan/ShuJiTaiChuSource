// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Weapon/SacraEnemyArrowProjectile.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Effect/Effectable.h"
#include "Game/Characters/BaseCharacter.h"
#include "Game/CollisionConfig.h"
#include "GameFramework/ProjectileMovementComponent.h"

ASacraEnemyArrowProjectile::ASacraEnemyArrowProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(RootSceneComponent);

	ArrowMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowMesh"));
	ArrowMeshComponent->SetupAttachment(RootSceneComponent);
	ArrowMeshComponent->SetCollisionProfileName(CP_NO_COLLISION);
	ArrowMeshComponent->SetSimulatePhysics(false);

	ArrowTipComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ArrowTip"));
	ArrowTipComponent->SetupAttachment(ArrowMeshComponent);
	ArrowTipComponent->SetRelativeLocation(FVector(30.0f, 0.0f, 0.0f));

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovementComponent->UpdatedComponent = ArrowMeshComponent;
	ProjectileMovementComponent->InitialSpeed = 0.0f;
	ProjectileMovementComponent->MaxSpeed = 5000.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bShouldBounce = false;
	ProjectileMovementComponent->ProjectileGravityScale = 1.0f;
	ProjectileMovementComponent->SetActive(false);
}

void ASacraEnemyArrowProjectile::BeginPlay()
{
	Super::BeginPlay();
	EnterLoadedState();
}

void ASacraEnemyArrowProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsInFlight && !bHasHit && IsValid(ProjectileMovementComponent) && ProjectileMovementComponent->IsActive())
	{
		PerformFlightTrace();
	}
}

void ASacraEnemyArrowProjectile::EnterLoadedState()
{
	bHasHit = false;
	bIsInFlight = false;
	PreviousTipLocation = ArrowTipComponent ? ArrowTipComponent->GetComponentLocation() : GetActorLocation();

	if (IsValid(ArrowMeshComponent))
	{
		ArrowMeshComponent->SetCollisionProfileName(CP_ARROW_NOCKED);
		ArrowMeshComponent->SetSimulatePhysics(false);
	}

	if (IsValid(ProjectileMovementComponent))
	{
		ProjectileMovementComponent->StopMovementImmediately();
		ProjectileMovementComponent->Deactivate();
		ProjectileMovementComponent->SetComponentTickEnabled(false);
	}
}

void ASacraEnemyArrowProjectile::LaunchProjectile(AActor* InInstigatorActor, const FVector& InLaunchVelocity)
{
	InstigatorActor = InInstigatorActor;
	InstigatorOwnerActor = IsValid(InInstigatorActor) ? InInstigatorActor->GetOwner() : nullptr;
	bHasHit = false;
	bIsInFlight = true;
	PreviousTipLocation = ArrowTipComponent ? ArrowTipComponent->GetComponentLocation() : GetActorLocation();

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	if (IsValid(ArrowMeshComponent))
	{
		ArrowMeshComponent->SetCollisionProfileName(CP_NO_COLLISION);
		ArrowMeshComponent->SetSimulatePhysics(false);
	}

	if (IsValid(ProjectileMovementComponent))
	{
		ProjectileMovementComponent->StopMovementImmediately();
		ProjectileMovementComponent->SetUpdatedComponent(ArrowMeshComponent);
		ProjectileMovementComponent->Velocity = InLaunchVelocity;
		ProjectileMovementComponent->SetComponentTickEnabled(true);
		ProjectileMovementComponent->Activate(true);
	}
}

void ASacraEnemyArrowProjectile::SetProjectileGravityScale(float InGravityScale)
{
	if (IsValid(ProjectileMovementComponent))
	{
		ProjectileMovementComponent->ProjectileGravityScale = InGravityScale;
	}
}

void ASacraEnemyArrowProjectile::PerformFlightTrace()
{
	const FVector CurrentTipLocation = ArrowTipComponent ? ArrowTipComponent->GetComponentLocation() : GetActorLocation();

	FHitResult HitResult;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SacraEnemyArrowFlightTrace), false, this);
	QueryParams.AddIgnoredActor(this);
	if (IsValid(InstigatorActor))
	{
		QueryParams.AddIgnoredActor(InstigatorActor);
	}
	if (IsValid(InstigatorOwnerActor))
	{
		QueryParams.AddIgnoredActor(InstigatorOwnerActor);
	}

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		PreviousTipLocation,
		CurrentTipLocation,
		TCC_PROJECTILE,
		QueryParams);

	PreviousTipLocation = CurrentTipLocation;

	if (bHit && IsValid(HitResult.GetActor()))
	{
		HandleHit(HitResult);
	}
}

void ASacraEnemyArrowProjectile::HandleHit(const FHitResult& HitResult)
{
	UE_LOG(LogTemp, Log, TEXT("SacraEnemy Arrow Hit Start Instigator=%s HitActor=%s Bone=%s"),
		*GetNameSafe(InstigatorActor.Get()),
		*GetNameSafe(HitResult.GetActor()),
		*HitResult.BoneName.ToString());

	if (bHasHit)
	{
		return;
	}

	bHasHit = true;
	bIsInFlight = false;

	if (IsValid(ProjectileMovementComponent))
	{
		ProjectileMovementComponent->StopMovementImmediately();
		ProjectileMovementComponent->Deactivate();
		ProjectileMovementComponent->SetComponentTickEnabled(false);
	}

	AActor* HitActor = HitResult.GetActor();
	UPrimitiveComponent* HitComponent = HitResult.GetComponent();

	const float TipOffset = ArrowTipComponent ? ArrowTipComponent->GetRelativeLocation().X : 30.0f;
	SetActorLocation(HitResult.ImpactPoint - GetActorForwardVector() * TipOffset);

	if (bStickOnHit && IsValid(HitComponent))
	{
		AttachToComponent(HitComponent, FAttachmentTransformRules::KeepWorldTransform, HitResult.BoneName);
	}

	if (IsValid(ArrowMeshComponent))
	{
		ArrowMeshComponent->SetCollisionProfileName(CP_ARROW_STUCK);
	}

	ApplyArrowEffect(HitActor);
	SetLifeSpan(LifeSecondsAfterHit);
}

void ASacraEnemyArrowProjectile::ApplyArrowEffect(AActor* HitActor) const
{
	if (!IsValid(HitActor) || HitActor == this || HitActor == InstigatorActor.Get() || HitActor == InstigatorOwnerActor.Get() || !HitActor->Implements<UEffectable>())
	{
		UE_LOG(LogTemp, Log, TEXT("SacraEnemy Arrow ApplyEffect Skipped Instigator=%s HitActor=%s (invalid/self/notEffectable)"),
			*GetNameSafe(InstigatorActor.Get()),
			*GetNameSafe(HitActor));
		return;
	}

	FEffect Effect;
	Effect.EffectTypes.Add(EEffectType::Arrow);
	Effect.Amount = ArrowDamage;
	Effect.Causer = const_cast<ASacraEnemyArrowProjectile*>(this);
	Effect.Instigator = Cast<ABaseCharacter>(InstigatorActor.Get());
	Effect.Duration = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy Arrow ApplyEffect DealingDamage Instigator=%s HitActor=%s Damage=%.1f"),
		*GetNameSafe(InstigatorActor.Get()),
		*GetNameSafe(HitActor),
		ArrowDamage);

	IEffectable::Execute_ApplyEffect(HitActor, Effect);
}
