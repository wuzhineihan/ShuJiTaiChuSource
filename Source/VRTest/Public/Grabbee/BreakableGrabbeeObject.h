// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Grabbee/GrabbeeObject.h"
#include "Effect/Effectable.h"
#include "BreakableGrabbeeObject.generated.h"

class USphereComponent;
class UGeometryCollectionComponent;

/**
 * Breakable physics prop (grabbee) inspired by old BP_Breakable_01.
 * - Breaks on strong hit against WorldStatic, or on specific effects (Arrow/Smash).
 */
UCLASS()
class VRTEST_API ABreakableGrabbeeObject : public AGrabbeeObject, public IEffectable
{
	GENERATED_BODY()

public:
	ABreakableGrabbeeObject();

	// IEffectable
	virtual void ApplyEffect_Implementation(const FEffect& Effect) override;

protected:
	UFUNCTION()
	void OnMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Breakable")
	void SpawnForceField(FTransform SpawnTransform);

	void Break();

public:
	// Components (match the BP layout: GeometryCollection/Audio/Sphere under the base mesh)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UGeometryCollectionComponent* GeometryCollection = nullptr;

	// Tuning
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breakable")
	float BreakImpulseThreshold = 1700.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breakable")
	float SmashDamageThreshold = 1700.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breakable")
	float DestroyDelaySeconds = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breakable")
	float NoiseRange = 1500.0f;

	// Optional: spawn an auxiliary field actor (content BP) to trigger Chaos fracture.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breakable")
	TSubclassOf<AActor> BreakFieldActorClass;

private:
	UPROPERTY()
	bool bBroken = false;
};
