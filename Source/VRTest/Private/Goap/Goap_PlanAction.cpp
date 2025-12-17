// Fill out your copyright notice in the Description page of Project Settings.


#include "../Public/Goap/Goap_PlanAction.h"

UGoap_PlanAction::UGoap_PlanAction()
{
	ActionName = "NULL";
	ActionCost = 0;
	ActionTag = FGameplayTag::RequestGameplayTag("EnemyAction");
}

UGoap_PlanAction::~UGoap_PlanAction(){}

//返回持续时间
float UGoap_PlanAction::Get_Duration()
{
	return duration;
}


FWorldState UGoap_PlanAction::ActionEffect(FWorldState CurrentWorldState)
{
	//默认是将前提条件的状态全部改为false，再将影响改为true，实际情况应该根据具体动作而定，子类可以重写这个函数
	for (const auto& StateName :PreCondition)
	{
		if (CurrentWorldState.WorldCheck.Contains(StateName))
		{
			CurrentWorldState.WorldCheck[StateName] = true;
		}
	}
	for (const auto& StateName :EffectState)
	{
		if (CurrentWorldState.WorldCheck.Contains(StateName))
		{
			CurrentWorldState.WorldCheck[StateName] = true;
		}
	}
	return CurrentWorldState;
}

TArray<FName> UGoap_PlanAction::CheckActionPreCondition(FWorldState* CurrentWorldState)
{
	TArray<FName> CurrentActionPreCondition;
	for (const auto& StateName :PreCondition)
	{
		if (CurrentWorldState->WorldCheck.Contains(StateName) && CurrentWorldState->WorldCheck[StateName] == false)
		{
			CurrentActionPreCondition.Add(StateName);
		}
	}
	return CurrentActionPreCondition;
}

bool UGoap_PlanAction::CanActionBeChosen(FWorldState* CurrentWorldState)
{
	//默认是只有所有条件都满足了之后才会添加，实际情况应该全部添加比较好，这个函数应该用不到
	//要用到的话再根据实际情况来添加
	bool ActionCheck=false;
	int StateCount=0;
	for (const auto& StateName :PreCondition)
	{
		if (CurrentWorldState->WorldCheck.Contains(StateName) && CurrentWorldState->WorldCheck[StateName] == true)
		{
			StateCount++;
		}
	}
	
	if (StateCount == PreCondition.Num())
	{
		ActionCheck=true;
	}else if (StateCount < PreCondition.Num())
	{
		ActionCheck=false;
	}else
	{
		UE_LOG(LogTemp,Error,TEXT("Invalid StateCount"));
	}
	
	return ActionCheck;
}

bool UGoap_PlanAction::CheckState(FName CurrentState, bool bCheck_, FWorldState* CurrentWorldState)
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

int UGoap_PlanAction::GetCost(FWorldState* CurrentWorldState)
{
	return ActionCost;
}


