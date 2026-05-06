// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"

#include "SacraBTService_IdleContext.generated.h"

UCLASS()
class VRTEST_API USacraBTService_IdleContext : public UBTService
{
	GENERATED_BODY()

public:
	USacraBTService_IdleContext();

protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	// ==================== Blackboard Output ====================

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector StandLocationKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector StandRotationKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector HasPatrolRouteKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IdleMoveSpeedKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector PatrolMoveSpeedKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsAtStandLocationKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsFacingStandRotationKey;

protected:
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0"))
	float StandLocationAcceptanceRadius = 50.0f;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float StandRotationYawTolerance = 5.0f;

private:
	// ==================== Internal Helpers ====================

	void CollectIdleContext(UBehaviorTreeComponent& OwnerComp) const;
};
