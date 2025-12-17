// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "BlueprintableSphereComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent),Blueprintable)
class VRTEST_API UBlueprintableSphereComponent : public USphereComponent
{
	GENERATED_BODY()
};
