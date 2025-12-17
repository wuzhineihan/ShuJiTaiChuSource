// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MyFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class VRTEST_API UMyFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(CallInEditor,BlueprintCallable)
	static void DestroyAIController(AAIController* AIControllerRef);
	
	UFUNCTION(CallInEditor,BlueprintCallable)
	static TArray<FVector> StarCalculateAdjacentPoints(FVector PlayerLocation,FVector StarLocation,float LineLength);
	UFUNCTION(CallInEditor,BlueprintCallable)
	static float CalculateRadius(FVector PlayerLocation,FVector ControllerLocation,FVector ControllerForwardVector,float Distance);
	UFUNCTION(CallInEditor, BlueprintCallable)
	static FVector CalculateProjectionPoint(FVector PlayerLocation,FVector ControllerLocation,FVector ControllerForwardVector,float Radius);
	UFUNCTION(CallInEditor, BlueprintCallable)
	static void FindNearestEnemy(UObject* WorldContextObject, FVector Location);
	UFUNCTION(CallInEditor,BlueprintCallable)
	static bool IsPositionReachable(UObject* WorldContextObject,FVector TargetPosition,FVector EnemyLocation);
};
