// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Effect/Effectable.h"
#include "GameplayTagContainer.h"
#include "HitNoiseMaker.generated.h"

class UStaticMeshComponent;
class UAudioSubsystem;

/**
 * 被击中会发声并制造噪音（吸引敌人注意）的场景物体。
 * 参考旧蓝图：BP_ActorCanBeHit_NoiseMaker
 */
UCLASS()
class VRTEST_API AHitNoiseMaker : public AActor, public IEffectable
{
	GENERATED_BODY()

public:
	AHitNoiseMaker();

	virtual void BeginPlay() override;

	// IEffectable
	virtual void ApplyEffect_Implementation(const FEffect& Effect) override;

	/** 手动触发（蓝图/关卡脚本也可以直接调用） */
	UFUNCTION(BlueprintCallable, Category = "Noise")
	void TriggerNoise();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float NoiseRange = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float NoiseLoudness = 1.0f;

	// Uses UAudioSubsystem + NormalSoundAsset mapping (see GameSettings).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (Categories = "NormalSound"))
	FGameplayTag HitSoundTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bLegacySetNearestEnemyCanHear = true;
};

