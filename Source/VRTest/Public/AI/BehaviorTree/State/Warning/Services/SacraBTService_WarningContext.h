// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"

#include "SacraBTService_WarningContext.generated.h"

UCLASS()
class VRTEST_API USacraBTService_WarningContext : public UBTService
{
	GENERATED_BODY()

public:
	USacraBTService_WarningContext();

protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	// ==================== Blackboard Input ====================

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector HasWarningLocationKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector WarningLocationKey;

	// ==================== Blackboard Output ====================

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector WarningMoveSpeedKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector DesiredMoveSpeedKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector WarningAnchorLocationKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector HasWarningAnchorLocationKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector HasSearchLocationKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector SearchLocationKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector HasReachedWarningAnchorKey;

private:
	// ==================== Internal Helpers ====================

	void CollectWarningContext(UBehaviorTreeComponent& OwnerComp) const;
};
