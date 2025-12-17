// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Goap/Goap_PlanAction.h"
#include "Action_Patrol.generated.h"

/**
 * 
 */
UCLASS()
class VRTEST_API UAction_Patrol : public UGoap_PlanAction
{
	GENERATED_BODY()
	UAction_Patrol();
	virtual FWorldState ActionEffect(FWorldState CurrentWorldState) override;
};
