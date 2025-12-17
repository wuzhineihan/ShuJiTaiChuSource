// Fill out your copyright notice in the Description page of Project Settings.


#include "Goap/Actions/Action_SearchLocation.h"

UAction_SearchLocation::UAction_SearchLocation()
{
	ActionName = "SearchLocation";
	ActionTag = FGameplayTag::RequestGameplayTag("EnemyAction.SearchLocation");
	ActionCost = 1;
	duration = 1.0f;
	Canbeinterrupted = true;
	NeedToMove = true;
	//No PreCondition
	EffectState.Add("ExecuteInvestigationMission");
}

TArray<FName> UAction_SearchLocation::CheckActionPreCondition(FWorldState* CurrentWorldState)
{
	return Super::CheckActionPreCondition(CurrentWorldState);
}

FWorldState UAction_SearchLocation::ActionEffect(FWorldState CurrentWorldState)
{
	if (CheckState("ExecuteInvestigationMission",true,&CurrentWorldState))
	{
		CurrentWorldState.WorldCheck["ExecuteInvestigationMission"] = false;
	}
	return CurrentWorldState;
}
