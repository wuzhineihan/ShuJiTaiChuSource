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
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure, Category = "Patrol")
	bool HasPatrolPoints() const;

	UFUNCTION(BlueprintCallable)
	FVector GetNextPatrolPointLocation(bool& bOutHasNextPoint);

	UFUNCTION(BlueprintPure)
	FVector GetCurrentPatrolPointLocation() const;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Patrol")
	bool Patrol=false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
	bool bLoopPatrol = true;

	UFUNCTION(BlueprintCallable, Category = "Patrol")
	bool AdvancePatrolPoint();

	UFUNCTION(BlueprintCallable, Category = "Patrol")
	void ResetPatrolIndex(int32 InPatrolPointIndex = 0);

private:
	int PatrolPointIndex = 0;

	UPROPERTY(Transient)
	bool bPatrolForward = true;
};
