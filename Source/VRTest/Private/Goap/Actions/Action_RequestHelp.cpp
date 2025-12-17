// Fill out your copyright notice in the Description page of Project Settings.


#include "Goap/Actions/Action_RequestHelp.h"

UAction_RequestHelp::UAction_RequestHelp()
{
	ActionName = "Action_RequestHelp";
	ActionTag = FGameplayTag::RequestGameplayTag("EnemyAction.RequestHelp");
	ActionCost = 2;
	duration = 1.0f;
	Canbeinterrupted = true;
	NeedToMove = false;
	PreCondition.Add("HasBugle");
	EffectState.Add("ExecuteKillEnemyMission");
	EffectState.Add("EnemyIsAlive");
}

TArray<FName> UAction_RequestHelp::CheckActionPreCondition(FWorldState* CurrentWorldState)
{
	TArray<FName> CurrentPreConditions;
	if (CheckState("HasBugle",false,CurrentWorldState))
	{
		CurrentPreConditions.Add("HasBugle");
	}
	return CurrentPreConditions;
}

FWorldState UAction_RequestHelp::ActionEffect(FWorldState CurrentWorldState)
{
	if (CheckState("EnemyIsAlive",true,&CurrentWorldState))
	{
		CurrentWorldState.WorldCheck["EnemyIsAlive"] = false;
	}
	if (CheckState("HasBugle",true,&CurrentWorldState))
	{
		CurrentWorldState.WorldCheck["HasBugle"] = false;
	}
	return CurrentWorldState;
}
