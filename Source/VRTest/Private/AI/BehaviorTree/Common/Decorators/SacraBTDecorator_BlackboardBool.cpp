// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTree/Common/Decorators/SacraBTDecorator_BlackboardBool.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"

USacraBTDecorator_BlackboardBool::USacraBTDecorator_BlackboardBool()
{
	NodeName = TEXT("Sacra Blackboard Bool");
	BlackboardKey.AddBoolFilter(this, TEXT("BlackboardKey"));
}

bool USacraBTDecorator_BlackboardBool::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComponent || BlackboardKey.SelectedKeyName.IsNone())
	{
		return false;
	}

	return BlackboardComponent->GetValueAsBool(BlackboardKey.SelectedKeyName) == bExpectedValue;
}
