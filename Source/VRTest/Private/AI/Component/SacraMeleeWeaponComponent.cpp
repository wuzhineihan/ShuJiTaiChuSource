// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Component/SacraMeleeWeaponComponent.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Effect/Effectable.h"
#include "Game/Characters/BaseCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

namespace
{
void EnableDroppedWeaponPhysics(ACharacter* OwnerCharacter, AActor* WeaponActor);
}

USacraMeleeWeaponComponent::USacraMeleeWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USacraMeleeWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	SyncRuntimeDurationsFromMontages();
}

bool USacraMeleeWeaponComponent::InitWeapon()
{
	if (IsWeaponInitialized())
	{
		return true;
	}

	CachedOwnerCharacter = Cast<ACharacter>(GetOwner());
	CachedOwnerMesh = CachedOwnerCharacter ? CachedOwnerCharacter->GetMesh() : nullptr;

	if (!CachedOwnerCharacter || !CachedOwnerMesh)
	{
		return false;
	}

	if (bCreateWeaponOnInit && !SpawnWeaponIfNeeded())
	{
		return false;
	}

	if (bCreateWeaponOnInit)
	{
		AttachWeaponToOwner();
	}

	SetWeaponInitialized(true);
	return true;
}

bool USacraMeleeWeaponComponent::EquipWeapon()
{
	if (!CachedOwnerCharacter && !InitWeapon())
	{
		return false;
	}

	if (!SpawnWeaponIfNeeded())
	{
		return false;
	}

	if (EquipMontage)
	{
		UAnimInstance* AnimInstance = GetOwnerAnimInstance();
		const float PlayedDuration = PlayOwnerMontage(EquipMontage);
		if (AnimInstance && PlayedDuration > 0.0f)
		{
			UE_LOG(LogTemp, Log, TEXT("SacraEnemy Weapon EquipMontageStarted Owner=%s Duration=%.2f Montage=%s"),
				*GetNameSafe(GetOwner()),
				PlayedDuration,
				*GetNameSafe(EquipMontage));

			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &USacraMeleeWeaponComponent::HandleEquipMontageEnded);
			AnimInstance->Montage_SetEndDelegate(EndDelegate, EquipMontage);
			return true;
		}

		UE_LOG(LogTemp, Warning, TEXT("SacraEnemy Weapon EquipMontageFailedToStart Owner=%s AnimInstance=%s Duration=%.2f Montage=%s Fallback=ImmediateEquip"),
			*GetNameSafe(GetOwner()),
			AnimInstance ? TEXT("Valid") : TEXT("Null"),
			PlayedDuration,
			*GetNameSafe(EquipMontage));

		return CompleteMeleeEquip();
	}

	return CompleteMeleeEquip();
}

void USacraMeleeWeaponComponent::UnequipWeapon()
{
	bIsAttackWindowOpen = false;
	bHasAppliedAttackHit = false;
	DetachWeaponFromOwner();
	Super::UnequipWeapon();
}

void USacraMeleeWeaponComponent::HandleOwnerDeath()
{
	if (IsAttacking())
	{
		FinishAttack(false);
	}

	DetachWeaponFromOwner();
	EnableDroppedWeaponPhysics(CachedOwnerCharacter.Get(), SpawnedWeaponActor.Get());
	Super::HandleOwnerDeath();
}

bool USacraMeleeWeaponComponent::StartAttack(AActor* InTargetActor)
{
	if (!CachedOwnerCharacter && !InitWeapon())
	{
		return false;
	}

	if (!Super::StartAttack(InTargetActor))
	{
		return false;
	}

	bIsAttackWindowOpen = false;
	bHasAppliedAttackHit = false;

	if (AttackMontage)
	{
		if (UAnimInstance* AnimInstance = GetOwnerAnimInstance())
		{
			const float PlayedDuration = PlayOwnerMontage(AttackMontage);
			if (PlayedDuration > 0.0f)
			{
				UE_LOG(LogTemp, Log, TEXT("SacraEnemy Weapon AttackMontageStarted Owner=%s Duration=%.2f Montage=%s"),
					*GetNameSafe(GetOwner()),
					PlayedDuration,
					*GetNameSafe(AttackMontage));

				FOnMontageEnded EndDelegate;
				EndDelegate.BindUObject(this, &USacraMeleeWeaponComponent::HandleAttackMontageEnded);
				AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackMontage);
				return true;
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("SacraEnemy Weapon AttackMontageFailedToStart Owner=%s Montage=%s Fallback=ImmediateFinish"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(AttackMontage));

		FinishAttack(true);
	}

	return true;
}

bool USacraMeleeWeaponComponent::IsTargetInAttackRange(const AActor* TargetActor, float DistanceToTarget) const
{
	if (!IsValid(TargetActor))
	{
		return false;
	}

	return DistanceToTarget >= MinAttackRange && DistanceToTarget <= MaxAttackRange;
}

bool USacraMeleeWeaponComponent::ShouldUseDirectApproachToTarget(const AActor* TargetActor, float DistanceToTarget) const
{
	return IsValid(TargetActor) && !IsTargetInAttackRange(TargetActor, DistanceToTarget);
}

bool USacraMeleeWeaponComponent::CompleteMeleeEquip()
{
	AttachWeaponToOwner();
	return CompleteEquipWeapon();
}

bool USacraMeleeWeaponComponent::BeginAttackWindow()
{
	if (!IsAttacking())
	{
		return false;
	}

	bIsAttackWindowOpen = true;
	return ApplyAttackHit();
}

bool USacraMeleeWeaponComponent::EndAttackWindow()
{
	bIsAttackWindowOpen = false;
	return true;
}

bool USacraMeleeWeaponComponent::ApplyAttackHit()
{
	UE_LOG(LogTemp, Log, TEXT("SacraEnemy Melee ApplyAttackHit Start Owner=%s IsAttacking=%s WindowOpen=%s AlreadyApplied=%s"),
		*GetNameSafe(GetOwner()),
		IsAttacking() ? TEXT("true") : TEXT("false"),
		bIsAttackWindowOpen ? TEXT("true") : TEXT("false"),
		bHasAppliedAttackHit ? TEXT("true") : TEXT("false"));

	if (!IsAttacking() || !bIsAttackWindowOpen || bHasAppliedAttackHit)
	{
		return false;
	}

	AActor* TargetActor = nullptr;
	if (!TryResolveAttackTarget(TargetActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("SacraEnemy Melee ApplyAttackHit Failed NoValidTarget Owner=%s"),
			*GetNameSafe(GetOwner()));
		return false;
	}

	if (!TargetActor->Implements<UEffectable>())
	{
		UE_LOG(LogTemp, Warning, TEXT("SacraEnemy Melee ApplyAttackHit Failed TargetNotEffectable Owner=%s Target=%s"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(TargetActor));
		return false;
	}

	FEffect Effect;
	Effect.EffectTypes.Add(EEffectType::Melee);
	Effect.Amount = AttackDamage;
	Effect.Causer = GetOwner();
	Effect.Instigator = Cast<ABaseCharacter>(CachedOwnerCharacter.Get());
	Effect.Duration = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy Melee ApplyAttackHit DealingDamage Owner=%s Target=%s Damage=%.1f"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(TargetActor),
		AttackDamage);

	IEffectable::Execute_ApplyEffect(TargetActor, Effect);
	bHasAppliedAttackHit = true;
	return true;
}

namespace
{
void ApplyOwnerWeaponCollisionIgnore(ACharacter* OwnerCharacter, AActor* WeaponActor)
{
	if (!IsValid(OwnerCharacter) || !IsValid(WeaponActor))
	{
		return;
	}

	OwnerCharacter->MoveIgnoreActorAdd(WeaponActor);

	TInlineComponentArray<UPrimitiveComponent*> WeaponPrimitiveComponents(WeaponActor);
	for (UPrimitiveComponent* PrimitiveComponent : WeaponPrimitiveComponents)
	{
		if (!IsValid(PrimitiveComponent))
		{
			continue;
		}

		PrimitiveComponent->SetSimulatePhysics(false);
		PrimitiveComponent->SetGenerateOverlapEvents(false);
		PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PrimitiveComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
		PrimitiveComponent->IgnoreActorWhenMoving(OwnerCharacter, true);
	}

	TInlineComponentArray<UPrimitiveComponent*> OwnerPrimitiveComponents(OwnerCharacter);
	for (UPrimitiveComponent* PrimitiveComponent : OwnerPrimitiveComponents)
	{
		if (!IsValid(PrimitiveComponent))
		{
			continue;
		}

		PrimitiveComponent->IgnoreActorWhenMoving(WeaponActor, true);
	}
}

void EnableDroppedWeaponPhysics(ACharacter* OwnerCharacter, AActor* WeaponActor)
{
	if (!IsValid(WeaponActor))
	{
		return;
	}

	if (IsValid(OwnerCharacter))
	{
		OwnerCharacter->MoveIgnoreActorRemove(WeaponActor);
	}

	TInlineComponentArray<UPrimitiveComponent*> WeaponPrimitiveComponents(WeaponActor);
	for (UPrimitiveComponent* PrimitiveComponent : WeaponPrimitiveComponents)
	{
		if (!IsValid(PrimitiveComponent))
		{
			continue;
		}

		PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		PrimitiveComponent->SetCollisionResponseToAllChannels(ECR_Block);
		PrimitiveComponent->SetGenerateOverlapEvents(true);
		PrimitiveComponent->SetSimulatePhysics(true);

		if (IsValid(OwnerCharacter))
		{
			PrimitiveComponent->IgnoreActorWhenMoving(OwnerCharacter, false);
		}
	}

	if (IsValid(OwnerCharacter))
	{
		TInlineComponentArray<UPrimitiveComponent*> OwnerPrimitiveComponents(OwnerCharacter);
		for (UPrimitiveComponent* PrimitiveComponent : OwnerPrimitiveComponents)
		{
			if (!IsValid(PrimitiveComponent))
			{
				continue;
			}

			PrimitiveComponent->IgnoreActorWhenMoving(WeaponActor, false);
		}
	}
}
}

bool USacraMeleeWeaponComponent::SpawnWeaponIfNeeded()
{
	if (IsValid(SpawnedWeaponActor))
	{
		return true;
	}

	if (!WeaponActorClass || !GetWorld() || !CachedOwnerMesh)
	{
		return false;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = GetOwner();
	SpawnParameters.Instigator = CachedOwnerCharacter;

	const FTransform SpawnTransform = CachedOwnerMesh->GetSocketTransform(AttachSocketName);
	SpawnedWeaponActor = GetWorld()->SpawnActor<AActor>(WeaponActorClass, SpawnTransform, SpawnParameters);
	if (!IsValid(SpawnedWeaponActor))
	{
		return false;
	}

	ApplyOwnerWeaponCollisionIgnore(CachedOwnerCharacter, SpawnedWeaponActor);

	return true;
}

void USacraMeleeWeaponComponent::AttachWeaponToOwner()
{
	if (!CachedOwnerCharacter && !InitWeapon())
	{
		return;
	}

	if (!SpawnWeaponIfNeeded() || !IsValid(CachedOwnerMesh))
	{
		return;
	}

	SpawnedWeaponActor->AttachToComponent(
		CachedOwnerMesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		AttachSocketName);

	ApplyOwnerWeaponCollisionIgnore(CachedOwnerCharacter, SpawnedWeaponActor);
}

void USacraMeleeWeaponComponent::DetachWeaponFromOwner()
{
	if (!IsValid(SpawnedWeaponActor))
	{
		return;
	}

	SpawnedWeaponActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
}

UAnimInstance* USacraMeleeWeaponComponent::GetOwnerAnimInstance() const
{
	return IsValid(CachedOwnerMesh) ? CachedOwnerMesh->GetAnimInstance() : nullptr;
}

float USacraMeleeWeaponComponent::PlayOwnerMontage(UAnimMontage* MontageToPlay) const
{
	if (!IsValid(CachedOwnerCharacter) || !MontageToPlay)
	{
		return 0.0f;
	}

	return CachedOwnerCharacter->PlayAnimMontage(MontageToPlay);
}

void USacraMeleeWeaponComponent::HandleEquipMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != EquipMontage)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy Weapon EquipMontageEnded Owner=%s Interrupted=%s"),
		*GetNameSafe(GetOwner()),
		bInterrupted ? TEXT("true") : TEXT("false"));

	if (bInterrupted)
	{
		BroadcastEquipFinishedMessage(false);
		return;
	}

	CompleteMeleeEquip();
}

void USacraMeleeWeaponComponent::HandleAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != AttackMontage || !IsAttacking())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy Weapon AttackMontageEnded Owner=%s Interrupted=%s"),
		*GetNameSafe(GetOwner()),
		bInterrupted ? TEXT("true") : TEXT("false"));

	if (!bInterrupted && !bHasAppliedAttackHit)
	{
		bIsAttackWindowOpen = true;
		ApplyAttackHit();
	}

	bIsAttackWindowOpen = false;
	FinishAttack(!bInterrupted);
}

bool USacraMeleeWeaponComponent::TryResolveAttackTarget(AActor*& OutTargetActor) const
{
	OutTargetActor = GetCurrentAttackTarget();
	if (!IsValid(OutTargetActor))
	{
		return false;
	}

	if (!IsValid(CachedOwnerCharacter))
	{
		return false;
	}

	const float DistanceToTarget = FVector::Dist(CachedOwnerCharacter->GetActorLocation(), OutTargetActor->GetActorLocation());
	return IsTargetInAttackRange(OutTargetActor, DistanceToTarget);
}

void USacraMeleeWeaponComponent::SyncRuntimeDurationsFromMontages()
{
	if (EquipMontage)
	{
		EquipDuration = EquipMontage->GetPlayLength();
	}

	if (AttackMontage)
	{
		AttackDuration = AttackMontage->GetPlayLength();
	}
}
