// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"

#include "SacraBTDecorator_BlackboardBool.generated.h"

UCLASS()
class VRTEST_API USacraBTDecorator_BlackboardBool : public UBTDecorator_BlackboardBase
{
	GENERATED_BODY()

public:
	USacraBTDecorator_BlackboardBool();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bExpectedValue = true;
};
