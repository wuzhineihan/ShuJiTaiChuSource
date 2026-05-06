// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"

#include "SacraBTService_AutoSetMoveSpeed.generated.h"

UCLASS()
class VRTEST_API USacraBTService_AutoSetMoveSpeed : public UBTService
{
	GENERATED_BODY()

public:
	USacraBTService_AutoSetMoveSpeed();

protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	// ==================== Config ====================

	// 从 Blackboard 中读取期望移动速度，并自动同步到 CharacterMovement。
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector DesiredMoveSpeedKey;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bOverrideMoveSpeed = false;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", EditCondition = "bOverrideMoveSpeed"))
	float OverrideMoveSpeed = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0"))
	float SpeedTolerance = 1.0f;

private:
	// ==================== Internal Helpers ====================

	void SyncMoveSpeed(UBehaviorTreeComponent& OwnerComp) const;
};
