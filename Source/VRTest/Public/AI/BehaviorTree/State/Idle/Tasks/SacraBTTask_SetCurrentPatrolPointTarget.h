// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"

#include "SacraBTTask_SetCurrentPatrolPointTarget.generated.h"

UCLASS()
class VRTEST_API USacraBTTask_SetCurrentPatrolPointTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	USacraBTTask_SetCurrentPatrolPointTarget();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	// ==================== Blackboard Input ====================

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector PatrolMoveSpeedKey;

	// ==================== Blackboard Output ====================

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector MoveTargetLocationKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector DesiredMoveSpeedKey;
};
