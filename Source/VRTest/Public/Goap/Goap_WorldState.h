// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Goap_WorldState.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FWorldState
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="WorldState")
	TMap<FName,bool> WorldCheck;

	UPROPERTY(editAnywhere, BlueprintReadWrite,Category="WorldState")
	TMap<FName,FVector> WorldPosition;
};
