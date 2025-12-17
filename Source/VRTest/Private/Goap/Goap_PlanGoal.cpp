// Fill out your copyright notice in the Description page of Project Settings.


#include "../Public/Goap/Goap_PlanGoal.h"

UGoap_PlanGoal::UGoap_PlanGoal(){}

UGoap_PlanGoal::~UGoap_PlanGoal(){}


float UGoap_PlanGoal::GetDiscontentment(float value)
{
	return value;
}

//这里的具体实现最好还是交给子类来实现，这样的话就会更加灵活
float UGoap_PlanGoal::GetCurrentvalue(FWorldState* CurrentWorldState)
{
	return goal_value;
}

TArray<FName> UGoap_PlanGoal::CheckGoalPreCondition(FWorldState* CurrentWorldState)
{
	return StateToChange;
}

bool UGoap_PlanGoal::CheckState(FName CurrentState, bool bCheck_, FWorldState* CurrentWorldState)
{
	bool bResult = false;
	if (CurrentWorldState->WorldCheck.Contains(CurrentState))
	{
		if (CurrentWorldState->WorldCheck[CurrentState] == bCheck_)
		{
			bResult = true;
		}
	}
	return bResult;
}
