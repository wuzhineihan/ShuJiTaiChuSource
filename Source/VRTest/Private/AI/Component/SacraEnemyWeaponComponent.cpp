// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Component/SacraEnemyWeaponComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "Game/MyGameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameplayMessageSubsystem.h"

USacraEnemyWeaponComponent::USacraEnemyWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USacraEnemyWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoInitOnBeginPlay)
	{
		InitWeapon();
	}
}

bool USacraEnemyWeaponComponent::InitWeapon()
{
	if (bIsWeaponInitialized)
	{
		return true;
	}

	SetWeaponInitialized(true);
	return true;
}

void USacraEnemyWeaponComponent::SetWeaponPaused(bool bInPaused)
{
	if (bIsWeaponPaused == bInPaused)
	{
		return;
	}

	bIsWeaponPaused = bInPaused;
	HandleWeaponPausedStateChanged();
}

bool USacraEnemyWeaponComponent::EquipWeapon()
{
	if (bIsWeaponPaused)
	{
		return false;
	}

	if (!bIsWeaponInitialized && !InitWeapon())
	{
		return false;
	}

	if (bIsWeaponEquipped)
	{
		return true;
	}

	return CompleteEquipWeapon();
}

bool USacraEnemyWeaponComponent::CompleteEquipWeapon()
{
	if (bIsWeaponPaused)
	{
		return false;
	}

	if (!bIsWeaponInitialized && !InitWeapon())
	{
		return false;
	}

	if (bIsWeaponEquipped)
	{
		return true;
	}

	SetWeaponEquippedState(true);
	BroadcastEquipFinishedMessage(true);

	return true;
}

void USacraEnemyWeaponComponent::BroadcastEquipFinishedMessage(bool bSuccess)
{
	if (UGameplayMessageSubsystem::HasInstance(this))
	{
		FEnemyWeaponEquipFinishedMessage Message;
		Message.WeaponComponent = this;
		Message.InstigatorActor = GetOwner();
		Message.bSuccess = bSuccess;

		UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
		MessageSubsystem.BroadcastMessage(MyProjectTags::TAG_AI_Message_Weapon_EquipFinished, Message);
	}
}

void USacraEnemyWeaponComponent::UnequipWeapon()
{
	SetWeaponEquippedState(false);
	bIsAttacking = false;
	CurrentAttackTarget = nullptr;
}

void USacraEnemyWeaponComponent::HandleOwnerDeath()
{
	SetWeaponPaused(true);
	UnequipWeapon();
}

bool USacraEnemyWeaponComponent::StartAttack(AActor* InTargetActor)
{
	if (bIsWeaponPaused)
	{
		return false;
	}

	if (!bIsWeaponInitialized && !InitWeapon())
	{
		return false;
	}

	if (!bIsWeaponEquipped || bIsAttacking || !IsValid(InTargetActor))
	{
		return false;
	}

	bIsAttacking = true;
	CurrentAttackTarget = InTargetActor;
	OnWeaponAttackStarted.Broadcast(InTargetActor);
	return true;
}

bool USacraEnemyWeaponComponent::IsTargetInAttackRange(const AActor* TargetActor, float DistanceToTarget) const
{
	return IsValid(TargetActor);
}

bool USacraEnemyWeaponComponent::CanAttackTarget(const AActor* TargetActor, float DistanceToTarget) const
{
	if (bIsWeaponPaused || bIsAttacking || !bIsWeaponEquipped)
	{
		return false;
	}

	return IsTargetInAttackRange(TargetActor, DistanceToTarget);
}

bool USacraEnemyWeaponComponent::ShouldKeepWeaponEquipped(float DistanceToTarget) const
{
	return true;
}

void USacraEnemyWeaponComponent::FinishAttack(bool bSuccess)
{
	if (!bIsAttacking)
	{
		return;
	}

	bIsAttacking = false;
	CurrentAttackTarget = nullptr;
	OnWeaponAttackFinished.Broadcast(bSuccess);

	if (UGameplayMessageSubsystem::HasInstance(this))
	{
		FEnemyWeaponAttackFinishedMessage Message;
		Message.WeaponComponent = this;
		Message.InstigatorActor = GetOwner();
		Message.bSuccess = bSuccess;

		UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
		MessageSubsystem.BroadcastMessage(MyProjectTags::TAG_AI_Message_Weapon_AttackFinished, Message);
	}
}

void USacraEnemyWeaponComponent::SetWeaponInitialized(bool bInInitialized)
{
	bIsWeaponInitialized = bInInitialized;
}

void USacraEnemyWeaponComponent::SetWeaponEquippedState(bool bInEquipped)
{
	if (bIsWeaponEquipped == bInEquipped)
	{
		return;
	}

	bIsWeaponEquipped = bInEquipped;
	ApplyWeaponAnimLayerState(bIsWeaponEquipped);
	OnWeaponEquippedChanged.Broadcast(bIsWeaponEquipped);
}

void USacraEnemyWeaponComponent::HandleWeaponPausedStateChanged()
{
	if (bIsWeaponPaused && bIsAttacking)
	{
		FinishAttack(false);
	}
}

void USacraEnemyWeaponComponent::ApplyWeaponAnimLayerState(bool bInEquipped) const
{
	USkeletalMeshComponent* OwnerMesh = ResolveOwnerMesh();
	if (!IsValid(OwnerMesh))
	{
		return;
	}

	if (bInEquipped)
	{
		if (EquippedAnimLayerClass)
		{
			OwnerMesh->LinkAnimClassLayers(EquippedAnimLayerClass);
		}

		return;
	}

	if (EquippedAnimLayerClass)
	{
		OwnerMesh->UnlinkAnimClassLayers(EquippedAnimLayerClass);
	}

	if (UnequippedAnimLayerClass)
	{
		OwnerMesh->LinkAnimClassLayers(UnequippedAnimLayerClass);
	}
}

USkeletalMeshComponent* USacraEnemyWeaponComponent::ResolveOwnerMesh() const
{
	const ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	return IsValid(OwnerCharacter) ? OwnerCharacter->GetMesh() : nullptr;
}
