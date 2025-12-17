// Fill out your copyright notice in the Description page of Project Settings.


#include "Goap/Goals/Goap_ExecutePatrolMission.h"

UGoap_ExecutePatrolMission::UGoap_ExecutePatrolMission()
{
	goal_name = "ExecutePatrolMission";
	goal_value = 0;
	ChangeOverTime = 0.0;
	StateToChange.Add("ExecutePatrolMission");
}

float UGoap_ExecutePatrolMission::GetCurrentvalue(FWorldState* CurrentWorldState)
{
	if (CurrentWorldState->WorldCheck.Contains("ExecutePatrolMission")&&CurrentWorldState->WorldCheck["ExecutePatrolMission"] == true)
	{
		goal_value = 1;
	}
	
	return goal_value;
}

TArray<FName> UGoap_ExecutePatrolMission::CheckGoalPreCondition(FWorldState* CurrentWorldState)
{
	TArray<FName> CurrentGoalPreConditions;
	if (CurrentWorldState->WorldCheck.Contains("ExecutePatrolMission"))
	{
		if (CurrentWorldState->WorldCheck["ExecutePatrolMission"] == true)
		{
			CurrentGoalPreConditions.Add("ExecutePatrolMission");
		}
	}
	
	return CurrentGoalPreConditions;
}
