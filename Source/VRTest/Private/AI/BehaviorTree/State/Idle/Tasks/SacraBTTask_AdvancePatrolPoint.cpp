// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTree/State/Idle/Tasks/SacraBTTask_AdvancePatrolPoint.h"

#include "AIController.h"
#include "AI/Component/SacraEnemyContextComponent.h"
#include "GameFramework/Pawn.h"

USacraBTTask_AdvancePatrolPoint::USacraBTTask_AdvancePatrolPoint()
{
	NodeName = TEXT("Sacra Advance Patrol Point");
}

EBTNodeResult::Type USacraBTTask_AdvancePatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	APawn* ControlledPawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	if (!ControlledPawn)
	{
		return EBTNodeResult::Failed;
	}

	USacraEnemyContextComponent* EnemyContextComponent = ControlledPawn->FindComponentByClass<USacraEnemyContextComponent>();
	if (!EnemyContextComponent)
	{
		return EBTNodeResult::Failed;
	}

	const bool bAdvanceSucceeded = EnemyContextComponent->AdvancePatrolPoint();
	UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT Idle AdvancePatrolPoint Owner=%s Result=%s"),
		*GetNameSafe(OwnerComp.GetAIOwner()),
		bAdvanceSucceeded ? TEXT("Succeeded") : TEXT("Failed"));

	return bAdvanceSucceeded ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}
