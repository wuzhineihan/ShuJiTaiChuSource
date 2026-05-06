// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"

#include "SacraBTTask_SetDirectFightApproachLocation.generated.h"

UCLASS()
class VRTEST_API USacraBTTask_SetDirectFightApproachLocation : public UBTTaskNode
{
	GENERATED_BODY()

public:
	USacraBTTask_SetDirectFightApproachLocation();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector FightTargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector FightTargetLocationKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector HasFightPositionKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector FightPositionKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector DesiredMoveSpeedKey;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0"))
	float PreferredDistanceFromTarget = 120.0f;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0"))
	float MoveSpeed = 350.0f;

private:
	void WriteFailureResult(UBlackboardComponent& BlackboardComponent) const;
	void WriteSuccessResult(UBlackboardComponent& BlackboardComponent, const FVector& FightPosition) const;
};
