// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StarDrawMainStar.generated.h"

class UStaticMeshComponent;

/** MainStar：轨迹经过的关键点（被确认的点）。 */
UCLASS(Blueprintable)
class VRTEST_API AStarDrawMainStar : public AActor
{
	GENERATED_BODY()

public:
	AStarDrawMainStar();

public:
	// MainStar 不需要碰撞，Mesh 作为根组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Skill|StarDraw")
	UStaticMeshComponent* Mesh = nullptr;
};
