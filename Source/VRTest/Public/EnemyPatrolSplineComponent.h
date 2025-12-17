// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "EnemyPatrolSplineComponent.generated.h"

/**
 * 
 */
UCLASS(meta=(BlueprintSpawnableComponent))
class VRTEST_API UEnemyPatrolSplineComponent : public USplineComponent
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void UpdatePatrolPoint();
	UFUNCTION(BlueprintCallable)
	FVector GetPatrolPointLocation();
	UFUNCTION(BlueprintCallable)
	bool CheckPatrolPointEnd();
	
	UPROPERTY(BlueprintReadwrite,EditAnywhere)
	bool Patrol=false;
private:
	int PatrolPointIndex=0;
};
