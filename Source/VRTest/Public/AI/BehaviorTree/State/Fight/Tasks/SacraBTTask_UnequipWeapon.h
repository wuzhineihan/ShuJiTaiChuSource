// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"

#include "SacraBTTask_UnequipWeapon.generated.h"

UCLASS()
class VRTEST_API USacraBTTask_UnequipWeapon : public UBTTaskNode
{
	GENERATED_BODY()

public:
	USacraBTTask_UnequipWeapon();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
