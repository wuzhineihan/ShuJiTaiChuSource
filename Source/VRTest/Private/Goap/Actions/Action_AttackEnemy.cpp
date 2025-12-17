// Fill out your copyright notice in the Description page of Project Settings.


#include "Goap/Actions/Action_AttackEnemy.h"

UAction_AttackEnemy::UAction_AttackEnemy()
{
	ActionName = "AttackEnemy";
	ActionTag = FGameplayTag::RequestGameplayTag("EnemyAction.AttackEnemy");
	ActionCost = 1;
	duration = 1.0f;
	Canbeinterrupted = false;
	NeedToMove = true;
	PreCondition.Add("PrepareToAttack");
	EffectState.Add("ExecuteKillEnemyMission");
	EffectState.Add("EnemyisAlive");
}

TArray<FName> UAction_AttackEnemy::CheckActionPreCondition(FWorldState* CurrentWorldState)
{
	TArray<FName> CurrentPreCondition;
	if (CheckState("PrepareToAttack",false,CurrentWorldState))
	{
		CurrentPreCondition.Add("PrepareToAttack");
	}
	return CurrentPreCondition;
}

FWorldState UAction_AttackEnemy::ActionEffect(FWorldState CurrentWorldState)
{
	/*if (CheckState("EnemyIsAlive",true,&CurrentWorldState))
	{
		CurrentWorldState.WorldCheck["EnemyisAlive"] = false;
	}*/
	if (CheckState("PrepareToAttack",true,&CurrentWorldState))
	{
		CurrentWorldState.WorldCheck["PrepareToAttack"] = false;
	}
	return CurrentWorldState;
}
