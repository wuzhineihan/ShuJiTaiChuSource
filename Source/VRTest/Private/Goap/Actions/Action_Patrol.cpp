// Fill out your copyright notice in the Description page of Project Settings.


#include "Goap/Actions/Action_Patrol.h"

UAction_Patrol::UAction_Patrol()
{
	PreCondition.Add("NeedToPatrol");
	EffectState.Add("ExecutePatrolMission");
	NeedToMove = false;
	ActionName = "Patrol";
	ActionCost = 1;
	ActionTag = FGameplayTag::RequestGameplayTag("EnemyAction.Patrol");
	Canbeinterrupted = true;
	duration = 1.0f;
	
}

FWorldState UAction_Patrol::ActionEffect(FWorldState CurrentWorldState)
{
	return CurrentWorldState;
}
