// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StarDrawFingerPoint.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class AStarDrawManager;

/**
 * FingerPoint：玩家“指向/手/相机输入源”投射到圆柱面上的当前位置。
 *
 * 蓝图里：
 * - 使用 SphereCollision 进行 overlap（OtherStars）
 * - Tick 中 SphereTraceSingleForObjects 检测触碰 OtherStars
 *
 * C++ 里：
 * - 主要用于携带碰撞体，触发与 OtherStars 的重叠回调
 */
UCLASS(Blueprintable)
class VRTEST_API AStarDrawFingerPoint : public AActor
{
	GENERATED_BODY()

public:
	AStarDrawFingerPoint();

	void SetDrawManager(AStarDrawManager* InManager) { DrawManager = InManager; }

	UFUNCTION(BlueprintCallable, Category = "Skill|StarDraw")
	void SetFingerWorldLocation(const FVector& NewLocation);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnFingerOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	/** Root：SphereCollision（可在蓝图中调半径/碰撞） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill|StarDraw")
	USphereComponent* SphereCollision = nullptr;

	/** 可视化 Mesh（蓝图子类可设置具体网格体） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill|StarDraw")
	UStaticMeshComponent* Mesh = nullptr;

protected:
	UPROPERTY()
	TObjectPtr<AStarDrawManager> DrawManager;
};
