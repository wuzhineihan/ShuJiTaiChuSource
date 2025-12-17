// Fill out your copyright notice in the Description page of Project Settings.


#include "Goap/Goals/Goap_ExecuteInvestigationMission.h"

#include "GeometryTypes.h"

UGoap_ExecuteInvestigationMission::UGoap_ExecuteInvestigationMission()
{
	goal_name = "ExecuteInvestigationMission";
	goal_value = 0;
	ChangeOverTime = 0.0;
	StateToChange.Add("ExecuteInvestigationMission");
}

float UGoap_ExecuteInvestigationMission::GetCurrentvalue(FWorldState* CurrentWorldState)
{
	if (CheckState("ExecuteInvestigationMission",true,CurrentWorldState))
	{
		goal_value = 2;
	}else
	{
		goal_value = 0;
	}
	return goal_value;
}

TArray<FName> UGoap_ExecuteInvestigationMission::CheckGoalPreCondition(FWorldState* CurrentWorldState)
{
	TArray<FName> CurrentPreConditions;
	if (CurrentWorldState->WorldCheck.Contains("ExecuteInvestigationMission"))
	{
		if (CurrentWorldState->WorldCheck["ExecuteInvestigationMission"] == true)
		{
			CurrentPreConditions.Add("ExecuteInvestigationMission");
		}
	}
	return CurrentPreConditions;
}
