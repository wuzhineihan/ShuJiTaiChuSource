// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"

#include "SacraBTTask_SetStandTarget.generated.h"

UCLASS()
class VRTEST_API USacraBTTask_SetStandTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	USacraBTTask_SetStandTarget();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	// ==================== Blackboard Input ====================

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector StandLocationKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector StandRotationKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IdleMoveSpeedKey;

	// ==================== Blackboard Output ====================

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector MoveTargetLocationKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector FacingRotationKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector DesiredMoveSpeedKey;
};
