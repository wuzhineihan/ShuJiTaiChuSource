// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "LadderVolumeComponent.generated.h"

UCLASS(ClassGroup=(Collision), Blueprintable, meta=(BlueprintSpawnableComponent))
class VRTEST_API ULadderVolumeComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	FVector GetLadderNormal() const;
};

