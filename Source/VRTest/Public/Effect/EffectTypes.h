// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EffectTypes.generated.h"

class ABaseCharacter;

UENUM(BlueprintType)
enum class EEffectType : uint8
{
	Arrow UMETA(DisplayName = "Arrow"),
	Smash UMETA(DisplayName = "Smash"),
	Melee UMETA(DisplayName = "Melee"),
	Fire UMETA(DisplayName = "Fire"),
	Stasis UMETA(DisplayName = "Stasis")
};

USTRUCT(BlueprintType)
struct FEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EEffectType> EffectTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Amount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* Causer = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ABaseCharacter* Instigator = nullptr;
};
