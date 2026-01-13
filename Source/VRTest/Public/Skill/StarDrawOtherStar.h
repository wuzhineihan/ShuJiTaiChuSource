// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Skill/StarDrawDirection.h"
#include "StarDrawOtherStar.generated.h"

class USceneComponent;
class USphereComponent;
class UStaticMeshComponent;

/**
 * OtherStar：围绕某个 MainStar 生成的 8 个候选点。
 * 蓝图里通过 ActorTag=OtherStars 判断；这里同样保留 Tag。
 */
UCLASS(Blueprintable)
class VRTEST_API AStarDrawOtherStar : public AActor
{
	GENERATED_BODY()

public:
	AStarDrawOtherStar();

	UFUNCTION(BlueprintCallable, Category="Skill|StarDraw")
	void SetDirection(EStarDrawDirection InDirection) { Direction = InDirection; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Skill|StarDraw")
	EStarDrawDirection GetDirection() const { return Direction; }

public:
	/** Root：SphereCollision */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Skill|StarDraw")
	USphereComponent* SphereCollision = nullptr;

	/** 可视化 Mesh（蓝图子类可设置具体网格体） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Skill|StarDraw")
	UStaticMeshComponent* Mesh = nullptr;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill|StarDraw")
	EStarDrawDirection Direction = EStarDrawDirection::Up;
};
