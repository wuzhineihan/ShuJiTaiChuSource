// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTree/State/Idle/Tasks/SacraBTTask_SetCurrentPatrolPointTarget.h"

#include "AIController.h"
#include "AI/Component/SacraEnemyContextComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "GameFramework/Pawn.h"

USacraBTTask_SetCurrentPatrolPointTarget::USacraBTTask_SetCurrentPatrolPointTarget()
{
	NodeName = TEXT("Sacra Set Current Patrol Point Target");

	PatrolMoveSpeedKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_SetCurrentPatrolPointTarget, PatrolMoveSpeedKey));
	MoveTargetLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_SetCurrentPatrolPointTarget, MoveTargetLocationKey));
	DesiredMoveSpeedKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_SetCurrentPatrolPointTarget, DesiredMoveSpeedKey));
}

EBTNodeResult::Type USacraBTTask_SetCurrentPatrolPointTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	APawn* ControlledPawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	if (!BlackboardComponent || !ControlledPawn)
	{
		return EBTNodeResult::Failed;
	}

	USacraEnemyContextComponent* EnemyContextComponent = ControlledPawn->FindComponentByClass<USacraEnemyContextComponent>();
	if (!EnemyContextComponent)
	{
		return EBTNodeResult::Failed;
	}

	FVector PatrolPointLocation = FVector::ZeroVector;
	if (!EnemyContextComponent->TryGetCurrentPatrolPoint(PatrolPointLocation))
	{
		return EBTNodeResult::Failed;
	}

	if (!MoveTargetLocationKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent->SetValueAsVector(MoveTargetLocationKey.SelectedKeyName, PatrolPointLocation);
	}

	if (!DesiredMoveSpeedKey.SelectedKeyName.IsNone() && !PatrolMoveSpeedKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent->SetValueAsFloat(DesiredMoveSpeedKey.SelectedKeyName, BlackboardComponent->GetValueAsFloat(PatrolMoveSpeedKey.SelectedKeyName));
	}

	const FString PatrolPointText = PatrolPointLocation.ToCompactString();
	const float DesiredMoveSpeed = !DesiredMoveSpeedKey.SelectedKeyName.IsNone()
		? BlackboardComponent->GetValueAsFloat(DesiredMoveSpeedKey.SelectedKeyName)
		: 0.0f;

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT Idle SetPatrolTarget Owner=%s PatrolPoint=%s DesiredMoveSpeed=%.2f"),
		*GetNameSafe(OwnerComp.GetAIOwner()),
		*PatrolPointText,
		DesiredMoveSpeed);

	return EBTNodeResult::Succeeded;
}
