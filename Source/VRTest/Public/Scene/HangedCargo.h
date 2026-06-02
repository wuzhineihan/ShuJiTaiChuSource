// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Scene/ArrowPassthrough.h"
#include "HangedCargo.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UPhysicsConstraintComponent;

UCLASS(Blueprintable)
class VRTEST_API AHangedCargo : public AActor, public IArrowPassthrough
{
	GENERATED_BODY()

public:
	AHangedCargo();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> DefaultSceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> InvisibleBlock;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Rope;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> RopeCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Cargo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPhysicsConstraintComponent> PhysicsConstraintUp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPhysicsConstraintComponent> PhysicsConstraintDown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cargo")
	float CargoDamage = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cargo")
	float MinImpactVelocity = 200.0f;

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ArrowPassthrough")
	void OnArrowPassThrough(AActor* Arrow);

	UFUNCTION()
	void OnCargoHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	void CutRope();

	bool bRopeCut = false;
};
