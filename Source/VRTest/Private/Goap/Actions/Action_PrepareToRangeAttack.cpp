// Fill out your copyright notice in the Description page of Project Settings.


#include "Goap/Actions/Action_PrepareToRangeAttack.h"

UAction_PrepareToRangeAttack::UAction_PrepareToRangeAttack()
{
	ActionName = "PrepareToRangeAttack";
	ActionTag = FGameplayTag::RequestGameplayTag("EnemyAction.PrepareToRangeAttack");
	ActionCost = 2;
	duration = 1.0f;
	Canbeinterrupted = true;
	NeedToMove = false;
	PreCondition.Add("BowEquipped");
	EffectState.Add("PrepareToAttack");
}

TArray<FName> UAction_PrepareToRangeAttack::CheckActionPreCondition(FWorldState* CurrentWorldState)
{
	TArray<FName> CurrentPreCondition;
	if (CheckState("BowEquipped",false,CurrentWorldState))
	{
		CurrentPreCondition.Add("BowEquipped");
	}
	return CurrentPreCondition;
}

FWorldState UAction_PrepareToRangeAttack::ActionEffect(FWorldState CurrentWorldState)
{
	if (CheckState("PrepareToAttack",false,&CurrentWorldState))
	{
		CurrentWorldState.WorldCheck["PrepareToAttack"] = true;
	}
	return CurrentWorldState;
}

int UAction_PrepareToRangeAttack::GetCost(FWorldState* CurrentWorldState)
{
	if (CheckState("EnemyIsNear",true,CurrentWorldState))
	{
		ActionCost = 4;
	}else
	{
		ActionCost = 2;
	}
	if (CheckState("EnemyIsFarAway",true,CurrentWorldState))
	{
		ActionCost = 1;
	}else
	{
		ActionCost = 2;
	}
	return ActionCost;
}
