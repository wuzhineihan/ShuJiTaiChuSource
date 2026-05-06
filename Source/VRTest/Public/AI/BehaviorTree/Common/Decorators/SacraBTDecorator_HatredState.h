// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Component/SacraEnemyHatredComponent.h"
#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"

#include "SacraBTDecorator_HatredState.generated.h"

UCLASS()
class VRTEST_API USacraBTDecorator_HatredState : public UBTDecorator_BlackboardBase
{
	GENERATED_BODY()

public:
	USacraBTDecorator_HatredState();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Condition")
	EHatredState ExpectedState = EHatredState::Idle;
};
