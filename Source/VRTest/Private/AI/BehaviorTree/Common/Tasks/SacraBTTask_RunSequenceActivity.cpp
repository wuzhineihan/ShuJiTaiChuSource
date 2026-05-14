// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTree/Common/Tasks/SacraBTTask_RunSequenceActivity.h"

#include "AIController.h"
#include "AI/Component/SacraEnemyActivityComponent.h"
#include "AI/Component/SacraEnemyContextComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "GameFramework/Pawn.h"
#include "LevelSequenceActor.h"

USacraBTTask_RunSequenceActivity::USacraBTTask_RunSequenceActivity()
{
	NodeName = TEXT("Sacra Run Sequence Activity");
	bCreateNodeInstance = true;

	SequenceActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_RunSequenceActivity, SequenceActorKey), ALevelSequenceActor::StaticClass());
}

EBTNodeResult::Type USacraBTTask_RunSequenceActivity::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UnbindActivityDelegate();

	APawn* ControlledPawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!IsValid(ControlledPawn) || !IsValid(BlackboardComponent) || SequenceActorKey.SelectedKeyName.IsNone())
	{
		return EBTNodeResult::Failed;
	}

	USacraEnemyContextComponent* EnemyContextComponent = ControlledPawn->FindComponentByClass<USacraEnemyContextComponent>();
	if (!IsValid(EnemyContextComponent))
	{
		return EBTNodeResult::Failed;
	}

	USacraEnemyActivityComponent* ActivityComponent = EnemyContextComponent->GetCachedActivityComponent();
	ALevelSequenceActor* SequenceActor = Cast<ALevelSequenceActor>(BlackboardComponent->GetValueAsObject(SequenceActorKey.SelectedKeyName));
	if (!IsValid(ActivityComponent) || !IsValid(SequenceActor))
	{
		return EBTNodeResult::Failed;
	}

	if (!ActivityComponent->StartSequenceActivity(SequenceActor))
	{
		return EBTNodeResult::Failed;
	}

	CachedOwnerComp = &OwnerComp;
	CachedActivityComponent = ActivityComponent;
	CachedActivityComponent->OnSpecialActivityChanged.AddDynamic(this, &USacraBTTask_RunSequenceActivity::HandleSpecialActivityChanged);

	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type USacraBTTask_RunSequenceActivity::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (IsValid(CachedActivityComponent))
	{
		CachedActivityComponent->StopCurrentActivity(false);
	}

	UnbindActivityDelegate();
	CachedOwnerComp = nullptr;
	CachedActivityComponent = nullptr;

	return Super::AbortTask(OwnerComp, NodeMemory);
}

void USacraBTTask_RunSequenceActivity::HandleSpecialActivityChanged(ESacraEnemySpecialActivityType ActivityType, bool bIsPlaying)
{
	if (ActivityType == ESacraEnemySpecialActivityType::Sequence && bIsPlaying)
	{
		return;
	}

	FinishTaskWithResult(EBTNodeResult::Succeeded);
}

void USacraBTTask_RunSequenceActivity::FinishTaskWithResult(EBTNodeResult::Type Result)
{
	UnbindActivityDelegate();

	UBehaviorTreeComponent* OwnerComp = CachedOwnerComp.Get();
	CachedOwnerComp = nullptr;
	CachedActivityComponent = nullptr;

	if (IsValid(OwnerComp))
	{
		FinishLatentTask(*OwnerComp, Result);
	}
}

void USacraBTTask_RunSequenceActivity::UnbindActivityDelegate()
{
	if (IsValid(CachedActivityComponent))
	{
		CachedActivityComponent->OnSpecialActivityChanged.RemoveDynamic(this, &USacraBTTask_RunSequenceActivity::HandleSpecialActivityChanged);
	}
}
