// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTree/State/Idle/Tasks/SacraBTTask_SetStandTarget.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Rotator.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

USacraBTTask_SetStandTarget::USacraBTTask_SetStandTarget()
{
	NodeName = TEXT("Sacra Set Stand Target");

	StandLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_SetStandTarget, StandLocationKey));
	StandRotationKey.AddRotatorFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_SetStandTarget, StandRotationKey));
	IdleMoveSpeedKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_SetStandTarget, IdleMoveSpeedKey));
	MoveTargetLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_SetStandTarget, MoveTargetLocationKey));
	FacingRotationKey.AddRotatorFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_SetStandTarget, FacingRotationKey));
	DesiredMoveSpeedKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_SetStandTarget, DesiredMoveSpeedKey));
}

EBTNodeResult::Type USacraBTTask_SetStandTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComponent)
	{
		return EBTNodeResult::Failed;
	}

	if (!MoveTargetLocationKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent->SetValueAsVector(MoveTargetLocationKey.SelectedKeyName, BlackboardComponent->GetValueAsVector(StandLocationKey.SelectedKeyName));
	}

	if (!FacingRotationKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent->SetValueAsRotator(FacingRotationKey.SelectedKeyName, BlackboardComponent->GetValueAsRotator(StandRotationKey.SelectedKeyName));
	}

	if (!DesiredMoveSpeedKey.SelectedKeyName.IsNone() && !IdleMoveSpeedKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent->SetValueAsFloat(DesiredMoveSpeedKey.SelectedKeyName, BlackboardComponent->GetValueAsFloat(IdleMoveSpeedKey.SelectedKeyName));
	}

	const FString MoveTargetText = !MoveTargetLocationKey.SelectedKeyName.IsNone()
		? BlackboardComponent->GetValueAsVector(MoveTargetLocationKey.SelectedKeyName).ToCompactString()
		: TEXT("None");
	const FString FacingRotationText = !FacingRotationKey.SelectedKeyName.IsNone()
		? BlackboardComponent->GetValueAsRotator(FacingRotationKey.SelectedKeyName).ToCompactString()
		: TEXT("None");
	const float DesiredMoveSpeed = !DesiredMoveSpeedKey.SelectedKeyName.IsNone()
		? BlackboardComponent->GetValueAsFloat(DesiredMoveSpeedKey.SelectedKeyName)
		: 0.0f;

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT Idle SetStandTarget Owner=%s MoveTarget=%s FacingRotation=%s DesiredMoveSpeed=%.2f"),
		*GetNameSafe(OwnerComp.GetAIOwner()),
		*MoveTargetText,
		*FacingRotationText,
		DesiredMoveSpeed);

	return EBTNodeResult::Succeeded;
}
