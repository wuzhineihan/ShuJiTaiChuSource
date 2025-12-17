// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Goap/Goap_PlanAction.h"
#include "Action_EquipRangeWeapon.generated.h"

/**
 * 
 */
UCLASS()
class VRTEST_API UAction_EquipRangeWeapon : public UGoap_PlanAction
{
	GENERATED_BODY()
	UAction_EquipRangeWeapon();
	virtual TArray<FName> CheckActionPreCondition(FWorldState* CurrentWorldState) override;
	virtual FWorldState ActionEffect(FWorldState CurrentWorldState) override;
	
};
