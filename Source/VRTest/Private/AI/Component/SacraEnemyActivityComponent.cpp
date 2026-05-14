// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Component/SacraEnemyActivityComponent.h"

#include "AI/SacraEnemyAIControllerBase.h"
#include "AI/Component/SacraEnemyHatredComponent.h"
#include "AI/Component/SacraEnemyWeaponComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"

USacraEnemyActivityComponent::USacraEnemyActivityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USacraEnemyActivityComponent::BeginPlay()
{
	Super::BeginPlay();

	ResolveRuntimeReferences();
	BindHatredDelegate();
}

void USacraEnemyActivityComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopCurrentActivity(false);
	UnbindHatredDelegate();

	Super::EndPlay(EndPlayReason);
}

bool USacraEnemyActivityComponent::StartSequenceActivity(ALevelSequenceActor* SequenceActor)
{
	if (!IsValid(SequenceActor))
	{
		return false;
	}

	ULevelSequencePlayer* SequencePlayer = SequenceActor->GetSequencePlayer();
	if (!IsValid(SequencePlayer))
	{
		return false;
	}

	if (CurrentSequenceActor == SequenceActor && CurrentActivityType == ESacraEnemySpecialActivityType::Sequence && SequencePlayer->IsPlaying())
	{
		return true;
	}

	StopCurrentActivity(false);
	ResolveRuntimeReferences();

	CurrentActivityType = ESacraEnemySpecialActivityType::Sequence;
	CurrentSequenceActor = SequenceActor;
	CurrentSequencePlayer = SequencePlayer;

	if (bDisableCameraCutsWhileSequence)
	{
		CurrentSequencePlayer->SetDisableCameraCuts(true);
	}

	ApplySequenceRuntimeLocks();
	BindSequencePlayerDelegate();
	CurrentSequencePlayer->Play();
	BroadcastActivityStateChanged();

	return true;
}

void USacraEnemyActivityComponent::StopCurrentActivity(bool bRestoreState)
{
	if (CurrentActivityType == ESacraEnemySpecialActivityType::None)
	{
		return;
	}

	ULevelSequencePlayer* SequencePlayer = CurrentSequencePlayer.Get();
	UnbindSequencePlayerDelegate();

	if (IsValid(SequencePlayer))
	{
		if (SequencePlayer->IsPlaying())
		{
			SequencePlayer->Stop();
		}

		if (bRestoreState)
		{
			SequencePlayer->RestoreState();
		}
	}

	ReleaseSequenceRuntimeLocks();
	ClearCurrentActivityState();
	BroadcastActivityStateChanged();
}

bool USacraEnemyActivityComponent::IsSpecialActivityPlaying() const
{
	if (CurrentActivityType == ESacraEnemySpecialActivityType::Sequence)
	{
		return IsValid(CurrentSequencePlayer) && CurrentSequencePlayer->IsPlaying();
	}

	return false;
}

void USacraEnemyActivityComponent::ResolveRuntimeReferences()
{
	if (!IsValid(CachedEnemyController))
	{
		if (const APawn* OwnerPawn = Cast<APawn>(GetOwner()))
		{
			CachedEnemyController = Cast<ASacraEnemyAIControllerBase>(OwnerPawn->GetController());
		}
	}

	if (!IsValid(CachedHatredComponent) && IsValid(CachedEnemyController))
	{
		CachedHatredComponent = CachedEnemyController->FindComponentByClass<USacraEnemyHatredComponent>();
	}

	if (!IsValid(CachedWeaponComponent) && GetOwner())
	{
		CachedWeaponComponent = GetOwner()->FindComponentByClass<USacraEnemyWeaponComponent>();
	}

	if (!IsValid(CachedMovementComponent))
	{
		if (const ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
		{
			CachedMovementComponent = OwnerCharacter->GetCharacterMovement();
		}
	}
}

void USacraEnemyActivityComponent::BindHatredDelegate()
{
	UnbindHatredDelegate();
	ResolveRuntimeReferences();

	if (IsValid(CachedHatredComponent))
	{
		CachedHatredComponent->OnHatredStateChanged.AddDynamic(this, &USacraEnemyActivityComponent::HandleHatredStateChanged);
	}
}

void USacraEnemyActivityComponent::UnbindHatredDelegate()
{
	if (IsValid(CachedHatredComponent))
	{
		CachedHatredComponent->OnHatredStateChanged.RemoveDynamic(this, &USacraEnemyActivityComponent::HandleHatredStateChanged);
	}
}

void USacraEnemyActivityComponent::BindSequencePlayerDelegate()
{
	UnbindSequencePlayerDelegate();

	if (IsValid(CurrentSequencePlayer))
	{
		CurrentSequencePlayer->OnFinished.AddDynamic(this, &USacraEnemyActivityComponent::HandleSequenceFinished);
	}
}

void USacraEnemyActivityComponent::UnbindSequencePlayerDelegate()
{
	if (IsValid(CurrentSequencePlayer))
	{
		CurrentSequencePlayer->OnFinished.RemoveDynamic(this, &USacraEnemyActivityComponent::HandleSequenceFinished);
	}
}

void USacraEnemyActivityComponent::BroadcastActivityStateChanged()
{
	OnSpecialActivityChanged.Broadcast(CurrentActivityType, IsSpecialActivityPlaying());
}

void USacraEnemyActivityComponent::ClearCurrentActivityState()
{
	CurrentActivityType = ESacraEnemySpecialActivityType::None;
	CurrentSequenceActor = nullptr;
	CurrentSequencePlayer = nullptr;
}

void USacraEnemyActivityComponent::ApplySequenceRuntimeLocks()
{
	ResolveRuntimeReferences();

	if (bPauseWeaponWhileSequence && IsValid(CachedWeaponComponent))
	{
		CachedWeaponComponent->SetWeaponPaused(true);
		bAppliedWeaponPause = true;
	}

	if (bDisableMovementWhileSequence && IsValid(CachedMovementComponent))
	{
		CachedMovementComponent->DisableMovement();
		CachedMovementComponent->StopMovementImmediately();
		bAppliedMovementLock = true;
	}
}

void USacraEnemyActivityComponent::ReleaseSequenceRuntimeLocks()
{
	if (bAppliedWeaponPause && IsValid(CachedWeaponComponent))
	{
		CachedWeaponComponent->SetWeaponPaused(false);
	}

	if (bAppliedMovementLock && IsValid(CachedMovementComponent) && CachedMovementComponent->MovementMode == MOVE_None)
	{
		CachedMovementComponent->SetMovementMode(MOVE_Walking);
	}

	bAppliedWeaponPause = false;
	bAppliedMovementLock = false;
}

void USacraEnemyActivityComponent::HandleHatredStateChanged(EHatredState NewState)
{
	if (NewState == EHatredState::Fight && bInterruptSequenceOnFight && CurrentActivityType == ESacraEnemySpecialActivityType::Sequence)
	{
		StopCurrentActivity(false);
	}
}

void USacraEnemyActivityComponent::HandleSequenceFinished()
{
	if (CurrentActivityType != ESacraEnemySpecialActivityType::Sequence)
	{
		return;
	}

	ReleaseSequenceRuntimeLocks();
	UnbindSequencePlayerDelegate();
	ClearCurrentActivityState();
	BroadcastActivityStateChanged();
}
