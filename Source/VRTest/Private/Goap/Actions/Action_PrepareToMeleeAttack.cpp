// Fill out your copyright notice in the Description page of Project Settings.


#include "Goap/Actions/Action_PrepareToMeleeAttack.h"

UAction_PrepareToMeleeAttack::UAction_PrepareToMeleeAttack()
{
	ActionName = "PrepareToAttack";
	ActionTag = FGameplayTag::RequestGameplayTag("EnemyAction.PrepareToMeleeAttack");
	ActionCost = 1;
	duration = 1.0f;
	Canbeinterrupted = true;
	NeedToMove = false;
	PreCondition.Add("MeleeEquipped");
	EffectState.Add("PrepareToAttack");
	
}

TArray<FName> UAction_PrepareToMeleeAttack::CheckActionPreCondition(FWorldState* CurrentWorldState)
{
	TArray<FName> CurrentPreCondition;
	if (CheckState("MeleeEquipped",false,CurrentWorldState))
	{
		CurrentPreCondition.Add("MeleeEquipped");
	}
	return CurrentPreCondition;
}

FWorldState UAction_PrepareToMeleeAttack::ActionEffect(FWorldState CurrentWorldState)
{
	if (CheckState("PrepareToAttack",false,&CurrentWorldState))
	{
		CurrentWorldState.WorldCheck["PrepareToAttack"] = true;
	}
	return CurrentWorldState;
}

int UAction_PrepareToMeleeAttack::GetCost(FWorldState* CurrentWorldState)
{
	if (CheckState("EnemyIsNear",true,CurrentWorldState))
	{
		ActionCost = 1;
	}else
	{
		ActionCost = 2;
	}
	if (CheckState("EnemyIsFarAway",true,CurrentWorldState))
	{
		ActionCost = 4;
	}else
	{
		ActionCost = 2;
	}
	return ActionCost;
}