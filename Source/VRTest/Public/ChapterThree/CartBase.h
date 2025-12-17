// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ArrowComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "CartBase.generated.h"

UCLASS()
class VRTEST_API ACartBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACartBase();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="ChasePoints")
	int ChasePointNums = 1;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="ChasePoints")
	TArray<USceneComponent*> ChasePoints;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="ChasePoints")
	float ChasePointSpacing = 450.f;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="ChasePoints")
	float ChasePointDistance = -1500.f;

	UPROPERTY(EditAnywhere,blueprintReadWrite,Category="ChasePoints")
	float ChasePointOffset = 0.f;
	
	UPROPERTY(Category = Cart,VisibleAnywhere,BlueprintReadOnly,meta = (AllowPrivateAccess=true))
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TObjectPtr<UArrowComponent> Arrow;
#endif
	
	UFUNCTION(BlueprintCallable)
	FVector GetChasePointLocation(int number);
	
	UFUNCTION(BlueprintCallable)
	USceneComponent* GetChasePoint(int number);

	UFUNCTION(BlueprintCallable)
	int GetChasePointNums();

	UFUNCTION(BlueprintCallable)
	USceneComponent* AssignChasePoint();

	UFUNCTION(BlueprintCallable)
	void RestitutionChasePoint(USceneComponent* ChasePoint);
	
	static FName StaticMeshComponentName;

	UPROPERTY()
	TArray<USceneComponent*> AssignedPoints;
	
	UPROPERTY()
	TArray<USphereComponent*> DebugPoints;
	
};
