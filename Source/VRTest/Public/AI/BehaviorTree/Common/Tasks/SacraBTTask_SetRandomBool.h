// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"

#include "SacraBTTask_SetRandomBool.generated.h"

UCLASS()
class VRTEST_API USacraBTTask_SetRandomBool : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	USacraBTTask_SetRandomBool();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Random", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TrueProbability = 0.5f;
};
