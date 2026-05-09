// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/SacraEnemyAIControllerBase.h"

#include "AI/Component/SacraBlackboardComponent.h"
#include "AI/DataAsset/SacraEnemyConfigDataAsset.h"
#include "AI/Component/SacraEnemyHatredComponent.h"
#include "AI/Component/SacraEnemyStatusUIComponent.h"
#include "AI/Component/SacraEnemyWeaponComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Game/Characters/SacraEnemy.h"
#include "Perception/AIPerceptionComponent.h"
#include "BrainComponent.h"

UAIPerceptionComponent* ASacraEnemyAIControllerBase::FindPerceptionComponent(AActor* Actor)
{
	if (IsValid(Actor))
	{
		return Actor->FindComponentByClass<UAIPerceptionComponent>();
	}
	return nullptr;
}

ASacraEnemyAIControllerBase::ASacraEnemyAIControllerBase()
{
	EnemyBlackboardComponent = CreateDefaultSubobject<USacraBlackboardComponent>(TEXT("EnemyBlackboardComponent"));
	EnemyHatredComponent = CreateDefaultSubobject<USacraEnemyHatredComponent>(TEXT("EnemyHatredComponent"));

	Blackboard = EnemyBlackboardComponent;
}

void ASacraEnemyAIControllerBase::EnsureRuntimeComponents()
{
	if (!IsValid(EnemyBlackboardComponent))
	{
		EnemyBlackboardComponent = FindComponentByClass<USacraBlackboardComponent>();
		if (!IsValid(EnemyBlackboardComponent))
		{
			EnemyBlackboardComponent = NewObject<USacraBlackboardComponent>(this, TEXT("EnemyBlackboardComponent_RuntimeFallback"));
			if (IsValid(EnemyBlackboardComponent))
			{
				AddInstanceComponent(EnemyBlackboardComponent);
				EnemyBlackboardComponent->RegisterComponent();
				UE_LOG(LogTemp, Warning, TEXT("SacraEnemy Controller RecreatedBlackboardComponent Owner=%s"), *GetNameSafe(this));
			}
		}
	}

	if (!IsValid(EnemyHatredComponent))
	{
		EnemyHatredComponent = FindComponentByClass<USacraEnemyHatredComponent>();
		if (!IsValid(EnemyHatredComponent))
		{
			EnemyHatredComponent = NewObject<USacraEnemyHatredComponent>(this, TEXT("EnemyHatredComponent_RuntimeFallback"));
			if (IsValid(EnemyHatredComponent))
			{
				AddInstanceComponent(EnemyHatredComponent);
				EnemyHatredComponent->RegisterComponent();
				UE_LOG(LogTemp, Warning, TEXT("SacraEnemy Controller RecreatedHatredComponent Owner=%s"), *GetNameSafe(this));
			}
		}
	}

	if (IsValid(EnemyBlackboardComponent))
	{
		Blackboard = EnemyBlackboardComponent;
	}
}

void ASacraEnemyAIControllerBase::BeginPlay()
{
	Super::BeginPlay();
	EnsureRuntimeComponents();
	BindHatredDelegates();

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy Controller BeginPlay Owner=%s Pawn=%s HasBlackboard=%s HasHatred=%s"),
		*GetNameSafe(this),
		*GetNameSafe(GetPawn()),
		IsValid(EnemyBlackboardComponent) ? TEXT("true") : TEXT("false"),
		IsValid(EnemyHatredComponent) ? TEXT("true") : TEXT("false"));

	if (IsValid(EnemyHatredComponent))
	{
		EnemyHatredComponent->InitHatredComponent();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SacraEnemy Controller MissingHatredComponent Owner=%s"), *GetNameSafe(this));
	}

	if (IsValid(EnemyBlackboardComponent))
	{
		EnemyBlackboardComponent->InitAutoCollect(EnemyHatredComponent);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SacraEnemy Controller MissingBlackboardComponent Owner=%s"), *GetNameSafe(this));
	}

	ApplyPausedStateToControllerComponents();
	ApplyPausedStateToPawnComponents();
	RefreshRotationMode();
	TryStartBehaviorTree();
}

void ASacraEnemyAIControllerBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindHatredDelegates();
	Super::EndPlay(EndPlayReason);
}

void ASacraEnemyAIControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	EnsureRuntimeComponents();

	if (const ASacraEnemy* SacraEnemy = Cast<ASacraEnemy>(InPawn))
	{
		ApplyConfigDataAsset(SacraEnemy->GetEnemyConfigDataAsset());
	}

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy Controller OnPossess Owner=%s Pawn=%s"),
		*GetNameSafe(this),
		*GetNameSafe(InPawn));

	ApplyPausedStateToPawnComponents();
	RefreshRotationMode();
	TryStartBehaviorTree();
}

void ASacraEnemyAIControllerBase::ApplyConfigDataAsset(const USacraEnemyConfigDataAsset* ConfigDataAsset)
{
	if (!IsValid(ConfigDataAsset))
	{
		return;
	}

	if (ConfigDataAsset->ControllerConfig.bOverrideControllerConfig)
	{
		if (ConfigDataAsset->ControllerConfig.DefaultBehaviorTreeAsset)
		{
			DefaultBehaviorTreeAsset = ConfigDataAsset->ControllerConfig.DefaultBehaviorTreeAsset;
		}

		NonFightRotationRateYaw = ConfigDataAsset->ControllerConfig.NonFightRotationRateYaw;
		FightRotationRateYaw = ConfigDataAsset->ControllerConfig.FightRotationRateYaw;
	}

	if (IsValid(EnemyHatredComponent))
	{
		EnemyHatredComponent->ApplyConfigData(ConfigDataAsset->HatredConfig);
	}

	RefreshRotationMode();
}

void ASacraEnemyAIControllerBase::SetEnemyAIPaused(bool bInPaused)
{
	if (bIsEnemyAIPaused == bInPaused)
	{
		return;
	}

	bIsEnemyAIPaused = bInPaused;

	if (bIsEnemyAIPaused)
	{
		StopMovement();
		if (BrainComponent && BrainComponent->IsRunning())
		{
			BrainComponent->PauseLogic(TEXT("SacraEnemySubsystem"));
		}
	}
	else if (BrainComponent)
	{
		BrainComponent->ResumeLogic(TEXT("SacraEnemySubsystem"));
	}

	ApplyPausedStateToControllerComponents();
	ApplyPausedStateToPawnComponents();
}

void ASacraEnemyAIControllerBase::TryStartBehaviorTree()
{
	EnsureRuntimeComponents();

	if (!IsValid(DefaultBehaviorTreeAsset) || !IsValid(GetPawn()))
	{
		return;
	}

	UBlackboardComponent* BlackboardComponent = EnemyBlackboardComponent;
	if (!UseBlackboard(DefaultBehaviorTreeAsset->BlackboardAsset, BlackboardComponent))
	{
		return;
	}

	Blackboard = BlackboardComponent;

	if (!BrainComponent || !BrainComponent->IsRunning())
	{
		RunBehaviorTree(DefaultBehaviorTreeAsset);
	}
}

void ASacraEnemyAIControllerBase::ApplyPausedStateToControllerComponents()
{
	if (IsValid(EnemyHatredComponent))
	{
		EnemyHatredComponent->SetHatredPaused(bIsEnemyAIPaused);
	}

	if (IsValid(EnemyBlackboardComponent))
	{
		EnemyBlackboardComponent->SetAutoCollectPaused(bIsEnemyAIPaused);
	}

	if (UAIPerceptionComponent* CachedPerceptionComponent = FindPerceptionComponent(this))
	{
		CachedPerceptionComponent->SetComponentTickEnabled(!bIsEnemyAIPaused);
	}
}

void ASacraEnemyAIControllerBase::ApplyPausedStateToPawnComponents()
{
	APawn* ControlledPawn = GetPawn();
	if (!IsValid(ControlledPawn))
	{
		return;
	}

	if (USacraEnemyStatusUIComponent* StatusUIComponent = ControlledPawn->FindComponentByClass<USacraEnemyStatusUIComponent>())
	{
		StatusUIComponent->SetStatusUIPaused(bIsEnemyAIPaused);
	}

	if (ACharacter* ControlledCharacter = Cast<ACharacter>(ControlledPawn))
	{
		if (UCharacterMovementComponent* MovementComponent = ControlledCharacter->GetCharacterMovement())
		{
			if (bIsEnemyAIPaused)
			{
				MovementComponent->DisableMovement();
				MovementComponent->StopMovementImmediately();
			}
			else if (MovementComponent->MovementMode == MOVE_None)
			{
				MovementComponent->SetMovementMode(MOVE_Walking);
			}
		}
	}

	USacraEnemyWeaponComponent* WeaponComponent = nullptr;
	if (const ASacraEnemy* SacraEnemy = Cast<ASacraEnemy>(ControlledPawn))
	{
		WeaponComponent = SacraEnemy->GetEnemyWeaponComponent();
	}

	if (!WeaponComponent)
	{
		WeaponComponent = ControlledPawn->FindComponentByClass<USacraEnemyWeaponComponent>();
	}

	if (WeaponComponent)
	{
		WeaponComponent->SetWeaponPaused(bIsEnemyAIPaused);
	}
}

void ASacraEnemyAIControllerBase::BindHatredDelegates()
{
	UnbindHatredDelegates();

	if (!IsValid(EnemyHatredComponent))
	{
		return;
	}

	EnemyHatredComponent->OnHatredStateChanged.AddDynamic(this, &ASacraEnemyAIControllerBase::HandleHatredStateChanged);
	EnemyHatredComponent->OnFightTargetChanged.AddDynamic(this, &ASacraEnemyAIControllerBase::HandleFightTargetChanged);
}

void ASacraEnemyAIControllerBase::UnbindHatredDelegates()
{
	if (!IsValid(EnemyHatredComponent))
	{
		return;
	}

	EnemyHatredComponent->OnHatredStateChanged.RemoveDynamic(this, &ASacraEnemyAIControllerBase::HandleHatredStateChanged);
	EnemyHatredComponent->OnFightTargetChanged.RemoveDynamic(this, &ASacraEnemyAIControllerBase::HandleFightTargetChanged);
}

void ASacraEnemyAIControllerBase::RefreshRotationMode()
{
	const bool bInFight = IsValid(EnemyHatredComponent) && EnemyHatredComponent->GetCurrentHatredState() == EHatredState::Fight;
	ApplyMovementRotationMode(!bInFight);
	ApplyFightFocus(bInFight && IsValid(EnemyHatredComponent) ? EnemyHatredComponent->GetCurrentFightTargetActor() : nullptr);
}

void ASacraEnemyAIControllerBase::ApplyMovementRotationMode(bool bFaceMovement) const
{
	ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn());
	UCharacterMovementComponent* MovementComponent = ControlledCharacter ? ControlledCharacter->GetCharacterMovement() : nullptr;
	if (!ControlledCharacter || !MovementComponent)
	{
		return;
	}

	ControlledCharacter->bUseControllerRotationYaw = !bFaceMovement;
	MovementComponent->bUseControllerDesiredRotation = !bFaceMovement;
	MovementComponent->bOrientRotationToMovement = bFaceMovement;
	MovementComponent->RotationRate = FRotator(0.0f, bFaceMovement ? NonFightRotationRateYaw : FightRotationRateYaw, 0.0f);
}

void ASacraEnemyAIControllerBase::ApplyFightFocus(AActor* FocusTarget) const
{
	if (FocusTarget)
	{
		const_cast<ASacraEnemyAIControllerBase*>(this)->SetFocus(FocusTarget, EAIFocusPriority::Gameplay);
		return;
	}

	const_cast<ASacraEnemyAIControllerBase*>(this)->ClearFocus(EAIFocusPriority::Gameplay);
}

void ASacraEnemyAIControllerBase::HandleHatredStateChanged(EHatredState NewState)
{
	RefreshRotationMode();
}

void ASacraEnemyAIControllerBase::HandleFightTargetChanged(AActor* NewTargetActor)
{
	if (IsValid(EnemyHatredComponent) && EnemyHatredComponent->GetCurrentHatredState() == EHatredState::Fight)
	{
		ApplyFightFocus(NewTargetActor);
	}
}
