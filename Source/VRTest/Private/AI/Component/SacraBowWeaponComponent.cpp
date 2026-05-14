// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Component/SacraBowWeaponComponent.h"

#include "AI/Weapon/SacraEnemyArrowProjectile.h"
#include "AI/Weapon/SacraEnemyBowActor.h"
#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

USacraBowWeaponComponent::USacraBowWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USacraBowWeaponComponent::BeginPlay()
{
	Super::BeginPlay();
	OnWeaponAttackFinished.AddDynamic(this, &USacraBowWeaponComponent::HandleWeaponAttackFinishedInternal);
}

void USacraBowWeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (OnWeaponAttackFinished.IsAlreadyBound(this, &USacraBowWeaponComponent::HandleWeaponAttackFinishedInternal))
	{
		OnWeaponAttackFinished.RemoveDynamic(this, &USacraBowWeaponComponent::HandleWeaponAttackFinishedInternal);
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(AttackCooldownTimerHandle);
	}

	CleanupSpawnedActors();
	Super::EndPlay(EndPlayReason);
}

void USacraBowWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	RefreshVisualState();
}

bool USacraBowWeaponComponent::InitWeapon()
{
	if (IsWeaponInitialized())
	{
		return true;
	}

	CachedOwnerCharacter = Cast<ACharacter>(GetOwner());
	CachedOwnerMesh = IsValid(CachedOwnerCharacter) ? CachedOwnerCharacter->GetMesh() : nullptr;
	if (!IsValid(CachedOwnerCharacter) || !IsValid(CachedOwnerMesh))
	{
		return false;
	}

	if (!SpawnBowIfNeeded())
	{
		return false;
	}

	AttachBowToStowedSocket();
	UpdateBowVisualPullState(0.0f);
	RefreshVisualState();
	RefreshVisualState();
	SetWeaponInitialized(true);
	return true;
}

bool USacraBowWeaponComponent::EquipWeapon()
{
	if (!CachedOwnerCharacter && !InitWeapon())
	{
		return false;
	}

	if (IsWeaponEquipped())
	{
		return true;
	}

	RefreshVisualState();

	if (EquipMontage)
	{
		if (UAnimInstance* AnimInstance = GetOwnerAnimInstance())
		{
			const float PlayedDuration = PlayOwnerMontage(EquipMontage);
			if (PlayedDuration > 0.0f)
			{
				FOnMontageEnded EndDelegate;
				EndDelegate.BindUObject(this, &USacraBowWeaponComponent::HandleEquipMontageEnded);
				AnimInstance->Montage_SetEndDelegate(EndDelegate, EquipMontage);
				return true;
			}
		}
	}

	return NotifyCompleteEquip();
}

void USacraBowWeaponComponent::UnequipWeapon()
{
	UpdateBowVisualPullState(0.0f);
	SetWeaponAimingState(false);
	PendingAttackTarget = nullptr;
	bPendingStartAttackCooldown = false;
	bHasReceivedAttackReleaseNotify = false;

	if (bDestroyLoadedArrowOnUnequip && IsValid(LoadedArrowProjectile))
	{
		LoadedArrowProjectile->Destroy();
		LoadedArrowProjectile = nullptr;
	}

	if (bHideBowWhenUnequipped && IsValid(SpawnedBowActor))
	{
		SpawnedBowActor->SetActorHiddenInGame(true);
	}
	else
	{
		AttachBowToStowedSocket();
	}

	Super::UnequipWeapon();
}

bool USacraBowWeaponComponent::StartAttack(AActor* InTargetActor)
{
	if (!CachedOwnerCharacter && !InitWeapon())
	{
		return false;
	}

	const float DistanceToTarget = IsValid(InTargetActor) && IsValid(CachedOwnerCharacter)
		? FVector::Dist(CachedOwnerCharacter->GetActorLocation(), InTargetActor->GetActorLocation())
		: 0.0f;
	if (!CanAttackTarget(InTargetActor, DistanceToTarget))
	{
		return false;
	}

	if (!Super::StartAttack(InTargetActor))
	{
		return false;
	}

	PendingAttackTarget = InTargetActor;
	bPendingStartAttackCooldown = false;
	bHasReceivedAttackReleaseNotify = false;
	SetWeaponAimingState(true);
	UpdateBowVisualPullState(1.0f);

	if (AAIController* AIController = Cast<AAIController>(CachedOwnerCharacter ? CachedOwnerCharacter->GetController() : nullptr))
	{
		AIController->StopMovement();
	}

	if (UCharacterMovementComponent* CharacterMovement = CachedOwnerCharacter ? CachedOwnerCharacter->GetCharacterMovement() : nullptr)
	{
		CharacterMovement->StopMovementImmediately();
	}

	if (AttackMontage)
	{
		if (UAnimInstance* AnimInstance = GetOwnerAnimInstance())
		{
			const float PlayedDuration = PlayOwnerMontage(AttackMontage);
			if (PlayedDuration > 0.0f)
			{
				FOnMontageEnded EndDelegate;
				EndDelegate.BindUObject(this, &USacraBowWeaponComponent::HandleAttackMontageEnded);
				AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackMontage);
			}
		}
	}

	return true;
}

bool USacraBowWeaponComponent::SpawnBowIfNeeded()
{
	if (IsValid(SpawnedBowActor))
	{
		return true;
	}

	if (!BowActorClass || !GetWorld() || !IsValid(CachedOwnerMesh))
	{
		return false;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = GetOwner();
	SpawnParameters.Instigator = CachedOwnerCharacter;

	const FTransform SpawnTransform = CachedOwnerMesh->GetSocketTransform(BowEquipSocketName);
	SpawnedBowActor = GetWorld()->SpawnActor<ASacraEnemyBowActor>(BowActorClass, SpawnTransform, SpawnParameters);
	return IsValid(SpawnedBowActor);
}

bool USacraBowWeaponComponent::ReloadArrowIfNeeded()
{
	if (IsValid(LoadedArrowProjectile))
	{
		return true;
	}

	if (!ArrowProjectileClass || !GetWorld() || !IsValid(CachedOwnerMesh))
	{
		return false;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = GetOwner();
	SpawnParameters.Instigator = CachedOwnerCharacter;

	const FTransform SpawnTransform = CachedOwnerMesh->GetSocketTransform(ArrowNockSocketName);
	LoadedArrowProjectile = GetWorld()->SpawnActor<ASacraEnemyArrowProjectile>(ArrowProjectileClass, SpawnTransform, SpawnParameters);
	if (!IsValid(LoadedArrowProjectile))
	{
		return false;
	}

	LoadedArrowProjectile->AttachToComponent(
		CachedOwnerMesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		ArrowNockSocketName);
	LoadedArrowProjectile->EnterLoadedState();
	LoadedArrowProjectile->SetProjectileGravityScale(ProjectileGravityScale);
	SyncLoadedArrowToBow();

	return true;
}

bool USacraBowWeaponComponent::IsTargetInAttackRange(const AActor* TargetActor, float DistanceToTarget) const
{
	if (!IsValid(TargetActor))
	{
		return false;
	}

	return DistanceToTarget >= MinAttackRange && DistanceToTarget <= MaxAttackRange;
}

bool USacraBowWeaponComponent::CanAttackTarget(const AActor* TargetActor, float DistanceToTarget) const
{
	if (!USacraEnemyWeaponComponent::CanAttackTarget(TargetActor, DistanceToTarget))
	{
		return false;
	}

	if (bIsAttackOnCooldown)
	{
		return false;
	}

	return IsTargetInAttackRange(TargetActor, DistanceToTarget);
}

bool USacraBowWeaponComponent::ShouldKeepWeaponEquipped(float DistanceToTarget) const
{
	return true;
}

bool USacraBowWeaponComponent::ShouldAimAtTarget(const AActor* TargetActor, float DistanceToTarget, bool bHasLineOfSight) const
{
	if (!IsValid(TargetActor) || bIsAttackOnCooldown)
	{
		return false;
	}

	return IsTargetInAttackRange(TargetActor, DistanceToTarget) && bHasLineOfSight;
}

bool USacraBowWeaponComponent::IsInAttackRecovery() const
{
	return bIsAttackOnCooldown;
}

void USacraBowWeaponComponent::SetWeaponAiming(bool bInAiming, AActor* InTargetActor)
{
	if (!IsWeaponEquipped())
	{
		SetWeaponAimingState(false);
		UpdateBowVisualPullState(0.0f);
		return;
	}

	SetWeaponAimingState(bInAiming);

	if (!bInAiming && !IsAttacking())
	{
		UpdateBowVisualPullState(0.0f);
	}
}

void USacraBowWeaponComponent::AttachBowToEquippedSocket()
{
	if (!CachedOwnerCharacter && !InitWeapon())
	{
		return;
	}

	if (!SpawnBowIfNeeded() || !IsValid(CachedOwnerMesh))
	{
		return;
	}

	SpawnedBowActor->AttachToComponent(
		CachedOwnerMesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		BowEquipSocketName);
	SpawnedBowActor->SetActorHiddenInGame(false);
	RefreshVisualState();
}

void USacraBowWeaponComponent::AttachBowToStowedSocket()
{
	if (!CachedOwnerCharacter && !InitWeapon())
	{
		return;
	}

	if (!SpawnBowIfNeeded() || !IsValid(CachedOwnerMesh))
	{
		return;
	}

	const FName TargetSocketName = BowStowedSocketName.IsNone() ? BowEquipSocketName : BowStowedSocketName;
	SpawnedBowActor->AttachToComponent(
		CachedOwnerMesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		TargetSocketName);

	SpawnedBowActor->SetActorHiddenInGame(bHideBowWhenUnequipped);

	RefreshVisualState();
}

void USacraBowWeaponComponent::CleanupSpawnedActors()
{
	if (IsValid(LoadedArrowProjectile))
	{
		LoadedArrowProjectile->Destroy();
		LoadedArrowProjectile = nullptr;
	}

	if (IsValid(SpawnedBowActor))
	{
		SpawnedBowActor->Destroy();
		SpawnedBowActor = nullptr;
	}
}

void USacraBowWeaponComponent::SyncLoadedArrowToBow() const
{
	if (!IsValid(LoadedArrowProjectile) || !IsValid(CachedOwnerMesh))
	{
		return;
	}

	const FVector ArrowSocketLocation = CachedOwnerMesh->GetSocketLocation(ArrowNockSocketName);
	FVector AimLocation = ArrowSocketLocation + CachedOwnerCharacter->GetActorForwardVector() * 100.0f;
	if (IsValid(SpawnedBowActor) && IsValid(SpawnedBowActor->GetStandardPoint()))
	{
		AimLocation = SpawnedBowActor->GetStandardPoint()->GetComponentLocation();
	}

	const FVector ArrowDirection = (AimLocation - ArrowSocketLocation).GetSafeNormal();
	const FRotator ArrowRotation = ArrowDirection.IsNearlyZero()
		? CachedOwnerMesh->GetSocketRotation(ArrowNockSocketName)
		: ArrowDirection.Rotation();

	LoadedArrowProjectile->SetActorLocationAndRotation(ArrowSocketLocation, ArrowRotation, false, nullptr, ETeleportType::TeleportPhysics);
}

void USacraBowWeaponComponent::RefreshVisualState() const
{
	if (!IsValid(CachedOwnerMesh))
	{
		return;
	}

	if (IsValid(LoadedArrowProjectile))
	{
		SyncLoadedArrowToBow();
	}

	if (IsValid(SpawnedBowActor))
	{
		const float PullAlpha = (IsWeaponAiming() || IsAttacking()) ? 1.0f : 0.0f;
		const FVector GrabLocation = ComputeBowStringGrabLocation(PullAlpha);
		SpawnedBowActor->SetStringPullState(PullAlpha, GrabLocation);
	}
}

UAnimInstance* USacraBowWeaponComponent::GetOwnerAnimInstance() const
{
	return IsValid(CachedOwnerMesh) ? CachedOwnerMesh->GetAnimInstance() : nullptr;
}

float USacraBowWeaponComponent::PlayOwnerMontage(UAnimMontage* MontageToPlay) const
{
	if (!IsValid(CachedOwnerCharacter) || !IsValid(MontageToPlay))
	{
		return 0.0f;
	}

	return CachedOwnerCharacter->PlayAnimMontage(MontageToPlay);
}

bool USacraBowWeaponComponent::ReleaseCurrentArrowAtTarget(AActor* InTargetActor)
{
	if (!IsValid(LoadedArrowProjectile) || !IsValid(CachedOwnerMesh))
	{
		return false;
	}

	const FVector StartLocation = CachedOwnerMesh->GetSocketLocation(ArrowNockSocketName);
	FVector LaunchVelocity = CachedOwnerCharacter ? CachedOwnerCharacter->GetActorForwardVector() * ArrowLaunchSpeed : FVector::ForwardVector * ArrowLaunchSpeed;

	if (IsValid(InTargetActor))
	{
		const FVector TargetLocation = GetPredictedTargetLocation(InTargetActor, StartLocation);
		if (bUseSuggestProjectileVelocity)
		{
			FVector SuggestedVelocity = FVector::ZeroVector;
			UGameplayStatics::FSuggestProjectileVelocityParameters ProjectileVelocityParams(this, StartLocation, TargetLocation, ArrowLaunchSpeed);
			ProjectileVelocityParams.OverrideGravityZ = GetWorld() ? GetWorld()->GetGravityZ() * ProjectileGravityScale : 0.0f;
			ProjectileVelocityParams.TraceOption = ESuggestProjVelocityTraceOption::DoNotTrace;
			ProjectileVelocityParams.ActorsToIgnore.Add(GetOwner());
			ProjectileVelocityParams.ActorsToIgnore.Add(LoadedArrowProjectile);
			if (UGameplayStatics::SuggestProjectileVelocity(ProjectileVelocityParams, SuggestedVelocity))
			{
				LaunchVelocity = SuggestedVelocity;
			}
			else
			{
				LaunchVelocity = (TargetLocation - StartLocation).GetSafeNormal() * ArrowLaunchSpeed;
			}
		}
		else
		{
			LaunchVelocity = (TargetLocation - StartLocation).GetSafeNormal() * ArrowLaunchSpeed;
		}
	}

	LoadedArrowProjectile->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	LoadedArrowProjectile->SetActorRotation(LaunchVelocity.Rotation());
	LoadedArrowProjectile->LaunchProjectile(GetOwner(), LaunchVelocity);
	LoadedArrowProjectile = nullptr;
	bPendingStartAttackCooldown = true;
	SetWeaponAimingState(false);
	UpdateBowVisualPullState(0.0f);
	return true;
}

bool USacraBowWeaponComponent::NotifyCompleteEquip()
{
	if (IsWeaponPaused())
	{
		return false;
	}

	AttachBowToEquippedSocket();
	return CompleteEquipWeapon();
}

bool USacraBowWeaponComponent::NotifyAttackRelease()
{
	if (!IsAttacking() || bHasReceivedAttackReleaseNotify)
	{
		return false;
	}

	bHasReceivedAttackReleaseNotify = true;

	const bool bReleased = ReleaseCurrentArrowAtTarget(PendingAttackTarget.Get());
	PendingAttackTarget = nullptr;
	return bReleased;
}

bool USacraBowWeaponComponent::NotifyLoadArrow()
{
	if (IsWeaponPaused() || !IsWeaponEquipped())
	{
		return false;
	}

	if (!CachedOwnerCharacter && !InitWeapon())
	{
		return false;
	}

	return ReloadArrowIfNeeded();
}

FVector USacraBowWeaponComponent::GetTargetAimLocation(AActor* InTargetActor) const
{
	if (!IsValid(InTargetActor))
	{
		return FVector::ZeroVector;
	}

	FVector ViewLocation = FVector::ZeroVector;
	FRotator ViewRotation = FRotator::ZeroRotator;
	InTargetActor->GetActorEyesViewPoint(ViewLocation, ViewRotation);
	if (!ViewLocation.IsNearlyZero())
	{
		return ViewLocation;
	}

	if (const ACharacter* TargetCharacter = Cast<ACharacter>(InTargetActor))
	{
		if (const UCapsuleComponent* CapsuleComponent = TargetCharacter->GetCapsuleComponent())
		{
			return InTargetActor->GetActorLocation() + FVector(0.0f, 0.0f, CapsuleComponent->GetScaledCapsuleHalfHeight() * 0.75f);
		}
	}

	return InTargetActor->GetActorLocation() + FVector(0.0f, 0.0f, FallbackTargetAimHeightOffset);
}

FVector USacraBowWeaponComponent::GetPredictedTargetLocation(AActor* InTargetActor, const FVector& StartLocation) const
{
	if (!IsValid(InTargetActor))
	{
		return FVector::ZeroVector;
	}

	FVector PredictedTargetLocation = GetTargetAimLocation(InTargetActor);
	if (!bLeadMovingTargets || ArrowLaunchSpeed <= KINDA_SMALL_NUMBER)
	{
		return PredictedTargetLocation;
	}

	const FVector TargetVelocity = InTargetActor->GetVelocity();
	if (TargetVelocity.IsNearlyZero())
	{
		return PredictedTargetLocation;
	}

	const float DistanceToTarget = FVector::Dist(StartLocation, PredictedTargetLocation);
	const float PredictionTime = FMath::Clamp(DistanceToTarget / ArrowLaunchSpeed, 0.0f, MaxLeadPredictionTime);
	PredictedTargetLocation += TargetVelocity * PredictionTime;
	return PredictedTargetLocation;
}

void USacraBowWeaponComponent::UpdateBowVisualPullState(float PullAlpha)
{
	if (!IsValid(SpawnedBowActor) || !IsValid(CachedOwnerMesh))
	{
		return;
	}

	const FVector GrabLocation = ComputeBowStringGrabLocation(PullAlpha);
	SpawnedBowActor->SetStringPullState(PullAlpha, GrabLocation);
}

FVector USacraBowWeaponComponent::ComputeBowStringGrabLocation(float PullAlpha) const
{
	if (!IsValid(CachedOwnerMesh))
	{
		return FVector::ZeroVector;
	}

	const FVector RestLocation = CachedOwnerMesh->GetSocketLocation(PullStringSocketName);
	if (PullAlpha <= KINDA_SMALL_NUMBER || BowStringPullDistance <= KINDA_SMALL_NUMBER)
	{
		return RestLocation;
	}

	const FVector PullDirection = IsValid(CachedOwnerCharacter)
		? -CachedOwnerCharacter->GetActorForwardVector().GetSafeNormal()
		: -CachedOwnerMesh->GetForwardVector().GetSafeNormal();

	return RestLocation + PullDirection * BowStringPullDistance * FMath::Clamp(PullAlpha, 0.0f, 1.0f);
}

void USacraBowWeaponComponent::HandleEquipMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != EquipMontage)
	{
		return;
	}

	if (bInterrupted)
	{
		BroadcastEquipFinishedMessage(false);
		return;
	}

	NotifyCompleteEquip();
}

void USacraBowWeaponComponent::HandleAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != AttackMontage || !IsAttacking())
	{
		return;
	}

	if (bInterrupted)
	{
		PendingAttackTarget = nullptr;
		FinishAttack(false);
		return;
	}

	if (!bHasReceivedAttackReleaseNotify)
	{
		PendingAttackTarget = nullptr;
		FinishAttack(false);
		return;
	}

	FinishAttack(true);
}

void USacraBowWeaponComponent::HandleAttackCooldownFinished()
{
	bIsAttackOnCooldown = false;
}

void USacraBowWeaponComponent::SetWeaponAimingState(bool bInAiming)
{
	bIsWeaponAiming = bInAiming;
}

void USacraBowWeaponComponent::HandleWeaponPausedStateChanged()
{
	if (IsWeaponPaused())
	{
		PendingAttackTarget = nullptr;
		UpdateBowVisualPullState(0.0f);
		SetWeaponAimingState(false);
		bHasReceivedAttackReleaseNotify = false;
	}

	Super::HandleWeaponPausedStateChanged();
}

void USacraBowWeaponComponent::HandleWeaponAttackFinishedInternal(bool bSuccess)
{
	PendingAttackTarget = nullptr;
	UpdateBowVisualPullState(0.0f);
	SetWeaponAimingState(false);
	bHasReceivedAttackReleaseNotify = false;

	if (bSuccess && bPendingStartAttackCooldown && AttackCooldownDuration > 0.0f && GetWorld())
	{
		bIsAttackOnCooldown = true;
		GetWorld()->GetTimerManager().ClearTimer(AttackCooldownTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(
			AttackCooldownTimerHandle,
			this,
			&USacraBowWeaponComponent::HandleAttackCooldownFinished,
			AttackCooldownDuration,
			false);
	}

	bPendingStartAttackCooldown = false;
}
