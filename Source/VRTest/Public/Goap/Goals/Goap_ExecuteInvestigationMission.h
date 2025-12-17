// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Goap/Goap_PlanGoal.h"
#include "Goap_ExecuteInvestigationMission.generated.h"

/**
 * 
 */
UCLASS()
class VRTEST_API UGoap_ExecuteInvestigationMission : public UGoap_PlanGoal
{
	GENERATED_BODY()
	UGoap_ExecuteInvestigationMission();
	virtual float GetCurrentvalue(FWorldState* CurrentWorldState) override;
	virtual TArray<FName> CheckGoalPreCondition(FWorldState* CurrentWorldState) override;
};
