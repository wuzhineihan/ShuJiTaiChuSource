// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FireObject.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class UNiagaraComponent;
class USphereComponent;
class UPrimitiveComponent;

UCLASS()
class VRTEST_API AFireObject : public AActor
{
	GENERATED_BODY()

public:
	AFireObject();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnFireSphereBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* Mesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPointLightComponent* FireLight = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNiagaraComponent* FireNiagara = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* FireSphere = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire|Interact")
	FName IgniteBySightComponentTag = FName(TEXT("Interact_FireIgnite"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire|Effect")
	float FireEffectAmount = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire|Effect")
	float FireEffectDuration = 5.0f;
};
