// Fill out your copyright notice in the Description page of Project Settings.


#include "Goap/Actions/Action_EquipMeleeWeapon.h"


UAction_EquipMeleeWeapon::UAction_EquipMeleeWeapon()
{
	ActionName = "EquipMeleeWeapon";
	ActionTag = FGameplayTag::RequestGameplayTag("EnemyAction.EquipMeleeWeapon");
	ActionCost = 1;
	duration = 1.0f;
	Canbeinterrupted = false;
	NeedToMove = false;
	PreCondition.Add("HasMelee");
	EffectState.Add("MeleeEquipped");
}

TArray<FName> UAction_EquipMeleeWeapon::CheckActionPreCondition(FWorldState* CurrentWorldState)
{
	TArray<FName> CurrentPreCondition;
	if (CheckState("HasMelee",false,CurrentWorldState))
	{
		CurrentPreCondition.Add("HasMelee");
	}
	return CurrentPreCondition;
}

FWorldState UAction_EquipMeleeWeapon::ActionEffect(FWorldState CurrentWorldState)
{
	if (CheckState("MeleeEquipped",false,&CurrentWorldState))
	{
		CurrentWorldState.WorldCheck["MeleeEquipped"] = true;
	}
	return CurrentWorldState;
}
