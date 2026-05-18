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
	virtual void OnRegister() override;

#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

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

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Patrol")
	void BakePatrolPointsFromCurrentSpline();

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Patrol")
	void RestorePatrolPointsFromCache();

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Patrol")
	void SetPatrolPointLocalLocations(const TArray<FVector>& InLocalPointLocations);

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Patrol")
	void SetPatrolPointWorldLocations(const TArray<FVector>& InWorldPointLocations);

	UFUNCTION(BlueprintPure, Category = "Patrol")
	const TArray<FVector>& GetPatrolPointLocalLocations() const { return SavedPatrolPointLocalLocations; }

private:
	void ApplyPatrolPointLocalLocations(const TArray<FVector>& InLocalPointLocations, bool bMarkDirty);

	int PatrolPointIndex = 0;

	UPROPERTY(EditInstanceOnly, Category = "Patrol")
	TArray<FVector> SavedPatrolPointLocalLocations;

	UPROPERTY(Transient)
	bool bPatrolForward = true;

	UPROPERTY(Transient)
	bool bIsRestoringPatrolPoints = false;
};
