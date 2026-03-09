// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grabber/IGrabbable.h"
#include "ClimbableVolume.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UPlayerGrabHand;

UCLASS()
class VRTEST_API AClimbableVolume : public AActor, public IGrabbable
{
	GENERATED_BODY()

public:
	AClimbableVolume();

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
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* Box = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* PreviewMesh = nullptr;
};

