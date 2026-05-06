// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTree/State/Warning/Tasks/SacraBTTask_SetWarningSearchTarget.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

USacraBTTask_SetWarningSearchTarget::USacraBTTask_SetWarningSearchTarget()
{
	NodeName = TEXT("Sacra Set Warning Search Target");

	HasSearchLocationKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_SetWarningSearchTarget, HasSearchLocationKey));
	SearchLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_SetWarningSearchTarget, SearchLocationKey));
	MoveTargetLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_SetWarningSearchTarget, MoveTargetLocationKey));
}

EBTNodeResult::Type USacraBTTask_SetWarningSearchTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComponent || SearchLocationKey.SelectedKeyName.IsNone() || MoveTargetLocationKey.SelectedKeyName.IsNone())
	{
		return EBTNodeResult::Failed;
	}

	if (!HasSearchLocationKey.SelectedKeyName.IsNone() && !BlackboardComponent->GetValueAsBool(HasSearchLocationKey.SelectedKeyName))
	{
		return EBTNodeResult::Failed;
	}

	const FVector SearchLocation = BlackboardComponent->GetValueAsVector(SearchLocationKey.SelectedKeyName);
	BlackboardComponent->SetValueAsVector(MoveTargetLocationKey.SelectedKeyName, SearchLocation);

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT Warning SetSearchTarget Owner=%s SearchLocation=%s"),
		*GetNameSafe(OwnerComp.GetAIOwner()),
		*SearchLocation.ToCompactString());

	return EBTNodeResult::Succeeded;
}
