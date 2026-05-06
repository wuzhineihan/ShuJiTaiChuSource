// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"

#include "SacraBTTask_ClearWarningSearchLocation.generated.h"

UCLASS()
class VRTEST_API USacraBTTask_ClearWarningSearchLocation : public UBTTaskNode
{
	GENERATED_BODY()

public:
	USacraBTTask_ClearWarningSearchLocation();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector HasSearchLocationKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector SearchLocationKey;
};
