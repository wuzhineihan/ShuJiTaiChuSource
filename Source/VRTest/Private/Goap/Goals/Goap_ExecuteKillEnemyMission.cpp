// Fill out your copyright notice in the Description page of Project Settings.


#include "Goap/Goals/Goap_ExecuteKillEnemyMission.h"

UGoap_ExecuteKillEnemyMission::UGoap_ExecuteKillEnemyMission()
{
	goal_name = "ExecuteKillEnemyMission";
	goal_value = 0;
	ChangeOverTime = 0.0f;
	StateToChange.Add("ExecuteKillEnemyMission");
	StateToChange.Add("EnemyIsAlive");
	StateToChange.Add("LostEnemy");
}

float UGoap_ExecuteKillEnemyMission::GetCurrentvalue(FWorldState* CurrentWorldState)
{
	if (CheckState("ExecuteKillEnemyMission",true,CurrentWorldState)&&CheckState("EnemyIsAlive",true,CurrentWorldState)&&CheckState("LostEnemy",false,CurrentWorldState))
	{
		goal_value = 3;
		UE_LOG(LogTemp,Error,TEXT("chosekillgoal"));
	}else
	{
		goal_value = 0;
	}
	return goal_value;
}

TArray<FName> UGoap_ExecuteKillEnemyMission::CheckGoalPreCondition(FWorldState* CurrentWorldState)
{
	TArray<FName> CurrentGoalPreConditions;
	if (CheckState("ExecuteKillEnemyMission",true,CurrentWorldState))
	{
		CurrentGoalPreConditions.Add("ExecuteKillEnemyMission");
	}
	if (CheckState("EnemyisAlive",true,CurrentWorldState))
	{
		CurrentGoalPreConditions.Add("EnemyisAlive");
	}
	return CurrentGoalPreConditions;
}
