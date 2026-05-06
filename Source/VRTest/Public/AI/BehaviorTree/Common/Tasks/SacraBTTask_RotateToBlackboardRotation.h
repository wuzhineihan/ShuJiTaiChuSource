// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"

#include "SacraBTTask_RotateToBlackboardRotation.generated.h"

UCLASS()
class VRTEST_API USacraBTTask_RotateToBlackboardRotation : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	USacraBTTask_RotateToBlackboardRotation();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	// ==================== Config ====================

	UPROPERTY(EditAnywhere, Category = "Rotation", meta = (ClampMin = "0.0"))
	float YawTolerance = 3.0f;
};
