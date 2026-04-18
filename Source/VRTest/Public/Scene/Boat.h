// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Boat.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class USplineComponent;

UCLASS()
class VRTEST_API ABoat : public AActor
{
	GENERATED_BODY()

public:
	ABoat();

	virtual void Tick(float DeltaSeconds) override;

	/**
	 * Start one-shot movement along the spline.
	 * - Non-looping spline only.
	 * - Calling again after started will be ignored.
	 */
	UFUNCTION(BlueprintCallable, Category = "Boat|Move")
	void StartMoveOnce();

protected:
	virtual void BeginPlay() override;

	void UpdateBoatTransformByDistance(float DistanceAlongSpline);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Root = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* Mesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USplineComponent* MoveSpline = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boat|Move", meta = (ClampMin = "1.0"))
	float MoveSpeed = 200.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Boat|Move")
	bool bIsMoving = false;

	UPROPERTY(BlueprintReadOnly, Category = "Boat|Move")
	bool bMoveTriggered = false;

	UPROPERTY(BlueprintReadOnly, Category = "Boat|Move")
	float TravelDistance = 0.0f;

	float CachedSplineLength = 0.0f;
	FTransform CachedSplineWorldTransform;
};
