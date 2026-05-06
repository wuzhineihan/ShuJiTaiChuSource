// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTree/State/Warning/Tasks/SacraBTTask_SetWarningAnchorTarget.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

USacraBTTask_SetWarningAnchorTarget::USacraBTTask_SetWarningAnchorTarget()
{
	NodeName = TEXT("Sacra Set Warning Anchor Target");

	WarningAnchorLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_SetWarningAnchorTarget, WarningAnchorLocationKey));
	MoveTargetLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_SetWarningAnchorTarget, MoveTargetLocationKey));
}

EBTNodeResult::Type USacraBTTask_SetWarningAnchorTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComponent || WarningAnchorLocationKey.SelectedKeyName.IsNone() || MoveTargetLocationKey.SelectedKeyName.IsNone())
	{
		return EBTNodeResult::Failed;
	}

	const FVector WarningAnchorLocation = BlackboardComponent->GetValueAsVector(WarningAnchorLocationKey.SelectedKeyName);
	BlackboardComponent->SetValueAsVector(
		MoveTargetLocationKey.SelectedKeyName,
		WarningAnchorLocation);

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT Warning SetAnchorTarget Owner=%s AnchorLocation=%s"),
		*GetNameSafe(OwnerComp.GetAIOwner()),
		*WarningAnchorLocation.ToCompactString());

	return EBTNodeResult::Succeeded;
}
