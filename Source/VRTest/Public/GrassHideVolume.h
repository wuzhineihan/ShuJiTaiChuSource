// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerVolume.h"
#include "GrassHideVolume.generated.h"

class UAudioSubsystem;

/**
 * 
 */
UCLASS()
class VRTEST_API AGrassHideVolume : public ATriggerVolume
{
	GENERATED_BODY()
public:
	AGrassHideVolume();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(Transient)
	TObjectPtr<UAudioSubsystem> CachedAudioSubsystem = nullptr;

private:
	UFUNCTION()
	void OnBeginOverlap(class AActor* OverlappedActor,class AActor* OtherActor);
	UFUNCTION()
	void OnEndOverlap(class AActor* OverlappedActor,class AActor* OtherActor);
};
