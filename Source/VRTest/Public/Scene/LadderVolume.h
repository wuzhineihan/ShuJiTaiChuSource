// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LadderVolume.generated.h"

class UBoxComponent;

UCLASS()
class VRTEST_API ALadderVolume : public AActor
{
	GENERATED_BODY()

public:
	ALadderVolume();

	UFUNCTION(BlueprintCallable, Category = "Ladder")
	FVector GetLadderNormal() const;

	UFUNCTION(BlueprintCallable, Category = "Ladder")
	UBoxComponent* GetLadderVolume() const { return LadderBox; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* LadderBox = nullptr;
};

