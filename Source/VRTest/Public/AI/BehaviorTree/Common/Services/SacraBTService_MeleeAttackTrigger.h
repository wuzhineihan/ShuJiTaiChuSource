// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"

#include "SacraBTService_MeleeAttackTrigger.generated.h"

UCLASS()
class VRTEST_API USacraBTService_MeleeAttackTrigger : public UBTService
{
	GENERATED_BODY()

public:
	USacraBTService_MeleeAttackTrigger();

protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector FightTargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector ShouldTriggerMeleeAttackKey;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0"))
	float TriggerDistance = 180.0f;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bRequireTargetInWeaponAttackRange = true;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bRequireLineOfSight = false;

	UPROPERTY(EditAnywhere, Category = "Config")
	TEnumAsByte<ECollisionChannel> LineOfSightTraceChannel = ECC_Visibility;

private:
	void UpdateTriggerState(UBehaviorTreeComponent& OwnerComp) const;
	bool HasLineOfSightToTarget(const APawn& ControlledPawn, const AActor& TargetActor) const;
};
