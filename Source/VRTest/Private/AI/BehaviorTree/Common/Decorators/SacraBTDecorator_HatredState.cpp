// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTree/Common/Decorators/SacraBTDecorator_HatredState.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Enum.h"

USacraBTDecorator_HatredState::USacraBTDecorator_HatredState()
{
	NodeName = TEXT("Sacra Hatred State");
	BlackboardKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTDecorator_HatredState, BlackboardKey), StaticEnum<EHatredState>());
}

bool USacraBTDecorator_HatredState::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComponent || BlackboardKey.SelectedKeyName.IsNone())
	{
		return false;
	}

	return BlackboardComponent->GetValueAsEnum(BlackboardKey.SelectedKeyName) == static_cast<uint8>(ExpectedState);
}
