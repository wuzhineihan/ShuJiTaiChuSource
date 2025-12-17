// Fill out your copyright notice in the Description page of Project Settings.


#include "Goap/Actions/Action_Stand.h"

UAction_Stand::UAction_Stand()
{
	PreCondition.Add("NeedToPatrol");
	EffectState.Add("ExecutePatrolMission");
	ActionCost = 1;
	NeedToMove = false;
	ActionName = "Stand";
	ActionTag = FGameplayTag::RequestGameplayTag("EnemyAction.Stand");
	duration = 1.0f;
	Canbeinterrupted = true;
}

TArray<FName> UAction_Stand::CheckActionPreCondition(FWorldState* CurrentWorldState)
{
	TArray<FName> CurrentActionPreCondition; 
	if (CurrentWorldState->WorldCheck.Contains("NeedToPatrol"))
	{
		if (CurrentWorldState->WorldCheck["NeedToPatrol"] == true)
		{
			CurrentActionPreCondition.Add("NeedToPatrol");
		}
	}
	return CurrentActionPreCondition;
}

FWorldState UAction_Stand::ActionEffect(FWorldState CurrentWorldState)
{
	return CurrentWorldState;
}
