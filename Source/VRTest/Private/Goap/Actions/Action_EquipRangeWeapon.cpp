// Fill out your copyright notice in the Description page of Project Settings.


#include "Goap/Actions/Action_EquipRangeWeapon.h"

UAction_EquipRangeWeapon::UAction_EquipRangeWeapon()
{
	ActionName = "EquipRangeWeapon";
	ActionTag = FGameplayTag::RequestGameplayTag("EnemyAction.EquipRangeWeapon");
	ActionCost = 1;
	duration = 1.0f;
	Canbeinterrupted = false;
	NeedToMove = false;
	PreCondition.Add("HasBow");
	EffectState.Add("BowEquipped");
}

TArray<FName> UAction_EquipRangeWeapon::CheckActionPreCondition(FWorldState* CurrentWorldState)
{
	TArray<FName> CurrentPreCondition;
	if (CheckState("HasBow",false,CurrentWorldState))
	{
		CurrentPreCondition.Add("HasBow");
	}
	return CurrentPreCondition;
}

FWorldState UAction_EquipRangeWeapon::ActionEffect(FWorldState CurrentWorldState)
{
	if (CheckState("BowEquipped",false,&CurrentWorldState))
	{
		CurrentWorldState.WorldCheck["BowEquipped"] = true;
	}
	return CurrentWorldState;
}
