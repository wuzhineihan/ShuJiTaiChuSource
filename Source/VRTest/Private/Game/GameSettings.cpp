// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/GameSettings.h"
#include "Grabbee/Bow.h"
#include "Grabbee/Arrow.h"

UGameSettings::UGameSettings()
{
}

UGameSettings* UGameSettings::Get()
{
	return GetMutableDefault<UGameSettings>();
}

TSubclassOf<ABow> UGameSettings::GetBowClass() const
{
	if (BowClass.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("GameSettings: BowClass is not set!"));
		return nullptr;
	}
	return BowClass.LoadSynchronous();
}

TSubclassOf<AArrow> UGameSettings::GetArrowClass() const
{
	if (ArrowClass.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("GameSettings: ArrowClass is not set!"));
		return nullptr;
	}
	return ArrowClass.LoadSynchronous();
}

