// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"

#include "SacraBTTask_SetBlackboardBool.generated.h"

UCLASS()
class VRTEST_API USacraBTTask_SetBlackboardBool : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	USacraBTTask_SetBlackboardBool();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	bool bValue = false;
};
