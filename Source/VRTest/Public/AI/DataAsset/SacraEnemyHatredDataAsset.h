// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "SacraEnemyHatredDataAsset.generated.h"

UCLASS(BlueprintType)
class VRTEST_API USacraEnemyHatredDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hatred|Config", meta = (ClampMin = "0.01"))
	float UpdateHatredValueInterval = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hatred|Config", meta = (ClampMin = "0.0"))
	float WarningStateDecayInterval = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hatred|Config", meta = (ClampMin = "0.0"))
	float FightStateDecayInterval = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hatred|Config", meta = (ClampMin = "0.0"))
	float SightLoseGraceInterval = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hatred|Config")
	float IdleSightGrowthBase = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hatred|Config")
	float WarningSightGrowthBase = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hatred|Config")
	float DefaultDecreaseHatredValueBase = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hatred|Config", meta = (ClampMin = "1.0"))
	float MaxHatredValue = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hatred|Config", meta = (ClampMin = "0.0"))
	float WarningStateThreshold = 50.0f;
};
