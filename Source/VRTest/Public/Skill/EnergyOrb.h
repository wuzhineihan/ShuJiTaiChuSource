// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grabber/IGrabbable.h"
#include "EnergyOrb.generated.h"

class USphereComponent;
class UNiagaraComponent;
class UPlayerGrabHand;

UCLASS()
class VRTEST_API AEnergyOrb : public AActor, public IGrabbable
{
	GENERATED_BODY()

public:
	AEnergyOrb();

	// IGrabbable
	virtual EGrabType GetGrabType_Implementation() const override;
	virtual UPrimitiveComponent* GetGrabPrimitive_Implementation() const override;
	virtual bool CanBeGrabbedBy_Implementation(const UPlayerGrabHand* Hand) const override;
	virtual bool CanBeGrabbedByGravityGlove_Implementation() const override;
	virtual bool SupportsDualHandGrab_Implementation() const override;
	virtual void OnGrabbed_Implementation(UPlayerGrabHand* Hand) override;
	virtual void OnReleased_Implementation(UPlayerGrabHand* Hand) override;
	virtual void OnGrabSelected_Implementation() override;
	virtual void OnGrabDeselected_Implementation() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* Sphere = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNiagaraComponent* IdleEffect = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNiagaraComponent* BurstEffect = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Energy", meta=(ClampMin="1"))
	int32 EnergyRestoreAmount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Energy", meta=(ClampMin="0.0"))
	float DestroyDelay = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Energy")
	bool bConsumed = false;
};

