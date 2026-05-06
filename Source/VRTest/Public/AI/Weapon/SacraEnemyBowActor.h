// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "SacraEnemyBowActor.generated.h"

class UMaterialInstanceDynamic;
class UStaticMeshComponent;
class USceneComponent;

UCLASS()
class VRTEST_API ASacraEnemyBowActor : public AActor
{
	GENERATED_BODY()

public:
	ASacraEnemyBowActor();

	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintCallable, Category = "AI|Weapon|Bow")
	void SetStringPullState(float InPullAlpha, const FVector& InGrabLocation);

	UFUNCTION(BlueprintPure, Category = "AI|Weapon|Bow")
	USceneComponent* GetStandardPoint() const { return StandardPoint; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> RootSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> StandardPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BowMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> StringMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Bow")
	TObjectPtr<UMaterialInterface> StringMaterial = nullptr;

private:
	void EnsureStringMaterialInstance();

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> StringMaterialInstance = nullptr;
};
