// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StarDrawMainStar.generated.h"

class USphereComponent;
class UStaticMeshComponent;

/** MainStar：轨迹经过的关键点（被确认的点）。 */
UCLASS(Blueprintable)
class VRTEST_API AStarDrawMainStar : public AActor
{
	GENERATED_BODY()

public:
	AStarDrawMainStar();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Skill|StarDraw")
	USphereComponent* SphereCollision = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Skill|StarDraw")
	UStaticMeshComponent* Mesh = nullptr;
};
