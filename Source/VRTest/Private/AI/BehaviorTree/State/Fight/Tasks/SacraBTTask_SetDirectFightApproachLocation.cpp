// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTree/State/Fight/Tasks/SacraBTTask_SetDirectFightApproachLocation.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "GameFramework/Pawn.h"

USacraBTTask_SetDirectFightApproachLocation::USacraBTTask_SetDirectFightApproachLocation()
{
	NodeName = TEXT("Sacra Set Direct Fight Approach Location");

	FightTargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_SetDirectFightApproachLocation, FightTargetActorKey), AActor::StaticClass());
	FightTargetLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_SetDirectFightApproachLocation, FightTargetLocationKey));
	HasFightPositionKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_SetDirectFightApproachLocation, HasFightPositionKey));
	FightPositionKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_SetDirectFightApproachLocation, FightPositionKey));
	DesiredMoveSpeedKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_SetDirectFightApproachLocation, DesiredMoveSpeedKey));
}

EBTNodeResult::Type USacraBTTask_SetDirectFightApproachLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	APawn* ControlledPawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	if (!BlackboardComponent || !ControlledPawn)
	{
		return EBTNodeResult::Failed;
	}

	AActor* FightTargetActor = nullptr;
	if (!FightTargetActorKey.SelectedKeyName.IsNone())
	{
		FightTargetActor = Cast<AActor>(BlackboardComponent->GetValueAsObject(FightTargetActorKey.SelectedKeyName));
	}

	FVector TargetLocation = FVector::ZeroVector;
	if (IsValid(FightTargetActor))
	{
		TargetLocation = FightTargetActor->GetActorLocation();
	}
	else if (!FightTargetLocationKey.SelectedKeyName.IsNone())
	{
		TargetLocation = BlackboardComponent->GetValueAsVector(FightTargetLocationKey.SelectedKeyName);
	}

	if (TargetLocation.IsNearlyZero())
	{
		WriteFailureResult(*BlackboardComponent);
		return EBTNodeResult::Failed;
	}

	FVector ApproachDirection = ControlledPawn->GetActorLocation() - TargetLocation;
	ApproachDirection.Z = 0.0f;
	ApproachDirection = ApproachDirection.GetSafeNormal();
	if (ApproachDirection.IsNearlyZero())
	{
		ApproachDirection = -ControlledPawn->GetActorForwardVector().GetSafeNormal2D();
	}

	const FVector FightPosition = TargetLocation + (ApproachDirection * PreferredDistanceFromTarget);
	WriteSuccessResult(*BlackboardComponent, FightPosition);

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT Fight SetDirectApproach Owner=%s Target=%s FightPosition=%s Distance=%.2f"),
		*GetNameSafe(OwnerComp.GetAIOwner()),
		*TargetLocation.ToCompactString(),
		*FightPosition.ToCompactString(),
		PreferredDistanceFromTarget);

	return EBTNodeResult::Succeeded;
}

void USacraBTTask_SetDirectFightApproachLocation::WriteFailureResult(UBlackboardComponent& BlackboardComponent) const
{
	if (!HasFightPositionKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent.SetValueAsBool(HasFightPositionKey.SelectedKeyName, false);
	}

	if (!FightPositionKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent.ClearValue(FightPositionKey.SelectedKeyName);
	}
}

void USacraBTTask_SetDirectFightApproachLocation::WriteSuccessResult(UBlackboardComponent& BlackboardComponent, const FVector& FightPosition) const
{
	if (!HasFightPositionKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent.SetValueAsBool(HasFightPositionKey.SelectedKeyName, true);
	}

	if (!FightPositionKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent.SetValueAsVector(FightPositionKey.SelectedKeyName, FightPosition);
	}

	if (!DesiredMoveSpeedKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent.SetValueAsFloat(DesiredMoveSpeedKey.SelectedKeyName, MoveSpeed);
	}
}
