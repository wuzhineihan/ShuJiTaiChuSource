// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyPatrolSplineComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "Carriage.generated.h"

UCLASS()
class VRTEST_API ACarriage : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACarriage();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,Category=Components,meta=(AllowPrivateAccess=true))
	TObjectPtr<UChildActorComponent> Horse;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,Category=Components,meta=(AllowPrivateAccess=true))
	TObjectPtr<UChildActorComponent> Cart;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,Category=Components,meta=(AllowPrivateAccess=true))
	TObjectPtr<UEnemyPatrolSplineComponent> PatrolSpline;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Carriage")
	float CartDistance = 250.f;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Carriage")
	float CartRotateSpeed = 300.f;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Carriage")
	bool bEnableShake = false;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Carriage")
	float CartShakeAmplitude = 2.0f;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Carriage")
	float CartShakeFrequency = 5.0f;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Carriage")
	float PatrolPointRange = 300.f;
	
	UFUNCTION(BlueprintCallable,Category="Carriage")
	void UpdateCartPosition(float DeltaTime);

	UFUNCTION(BlueprintCallable,Category="Carriage")
	void SetMovable(bool Movable);

	UFUNCTION(BlueprintCallable,BlueprintImplementableEvent,Category="Carriage")
	void SetHorseMovable(bool Moveable);
	
	UFUNCTION(BlueprintCallable,Category="Carriage")
	void CheckPatrolState();

	UFUNCTION(BlueprintCallable,Category="Carriage")
	FVector GetCurrentCarriageLocation();

	UFUNCTION(BlueprintCallable,Category="Carriage")
	void ArriveFinalLocation();
	
	bool bMovable = false;
	static FName HorseName;
	static FName CartName;
	static FName PatrolSplineName;
	UPROPERTY(BlueprintReadOnly,category="Carriage")
	AActor* HorseActor;
	UPROPERTY(BlueprintReadOnly,Category="Carriage")
	AActor* CartActor;
	UPROPERTY(BlueprintReadOnly,Category="Carriage")
	bool bArriveFinalLocation = false;
};
