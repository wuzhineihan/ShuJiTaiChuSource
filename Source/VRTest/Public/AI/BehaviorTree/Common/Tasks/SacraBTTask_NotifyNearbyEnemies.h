// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AI/Component/SacraEnemyHatredComponent.h"

#include "SacraBTTask_NotifyNearbyEnemies.generated.h"

UCLASS()
class VRTEST_API USacraBTTask_NotifyNearbyEnemies : public UBTTaskNode
{
	GENERATED_BODY()

public:
	USacraBTTask_NotifyNearbyEnemies();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector WarningLocationKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector FightTargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0"))
	float NotifyRadius = 1200.0f;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bUseOwnerLocationWhenNoWarningLocation = true;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bAffectIdle = true;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bAffectWarning = true;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bAffectFight = false;
};
