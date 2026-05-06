// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "SacraEnemyArrowProjectile.generated.h"

class UProjectileMovementComponent;
class UPrimitiveComponent;
class UStaticMeshComponent;
class USceneComponent;

UCLASS()
class VRTEST_API ASacraEnemyArrowProjectile : public AActor
{
	GENERATED_BODY()

public:
	ASacraEnemyArrowProjectile();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "AI|Weapon|Arrow")
	void LaunchProjectile(AActor* InInstigatorActor, const FVector& InLaunchVelocity);

	UFUNCTION(BlueprintCallable, Category = "AI|Weapon|Arrow")
	void SetProjectileGravityScale(float InGravityScale);

	UFUNCTION(BlueprintCallable, Category = "AI|Weapon|Arrow")
	void EnterLoadedState();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> RootSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ArrowMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> ArrowTipComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Arrow", meta = (ClampMin = "0.0"))
	float ArrowDamage = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Arrow", meta = (ClampMin = "0.0"))
	float LifeSecondsAfterHit = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Arrow")
	bool bStickOnHit = true;

private:
	void PerformFlightTrace();
	void HandleHit(const FHitResult& HitResult);
	void ApplyArrowEffect(AActor* HitActor) const;

	UPROPERTY(Transient)
	TObjectPtr<AActor> InstigatorActor = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<AActor> InstigatorOwnerActor = nullptr;

	UPROPERTY(Transient)
	bool bHasHit = false;

	UPROPERTY(Transient)
	bool bIsInFlight = false;

	FVector PreviousTipLocation = FVector::ZeroVector;
};
