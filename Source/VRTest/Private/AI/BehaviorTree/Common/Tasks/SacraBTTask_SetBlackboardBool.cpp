// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTree/Common/Tasks/SacraBTTask_SetBlackboardBool.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"

USacraBTTask_SetBlackboardBool::USacraBTTask_SetBlackboardBool()
{
	NodeName = TEXT("Sacra Set Blackboard Bool");

	BlackboardKey.AddBoolFilter(this, TEXT("BlackboardKey"));
}

EBTNodeResult::Type USacraBTTask_SetBlackboardBool::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComponent || GetSelectedBlackboardKey().IsNone())
	{
		return EBTNodeResult::Failed;
	}

	BlackboardComponent->SetValueAsBool(GetSelectedBlackboardKey(), bValue);

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT SetBlackboardBool Owner=%s Key=%s Value=%s"),
		*GetNameSafe(OwnerComp.GetAIOwner()),
		*GetSelectedBlackboardKey().ToString(),
		bValue ? TEXT("true") : TEXT("false"));

	return EBTNodeResult::Succeeded;
}
