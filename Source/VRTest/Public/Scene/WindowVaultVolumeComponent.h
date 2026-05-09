// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "WindowVaultVolumeComponent.generated.h"

UCLASS(ClassGroup=(Collision), Blueprintable, meta=(BlueprintSpawnableComponent))
class VRTEST_API UWindowVaultVolumeComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	UWindowVaultVolumeComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vault")
	float FrontBackOffset = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vault")
	float ApexExtraZ = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vault", meta=(ClampMin="0.01"))
	float PreAlignDuration = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vault", meta=(ClampMin="0.01"))
	float ToApexDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vault", meta=(ClampMin="0.01"))
	float ToLandDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vault", meta=(ClampMin="10.0"))
	float GroundTraceDistance = 500.0f;
};

