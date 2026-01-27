#pragma once

#include "CoreMinimal.h"
#include "Audio/AudioNormalSoundAsset.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AudioSubsystem.generated.h"

class USoundBase;

/**
 * GameInstance 级别的音频子系统：提供统一的便捷播放接口。
 */
UCLASS()
class VRTEST_API UAudioSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

protected:
	
	UPROPERTY(Transient)
	TObjectPtr<UAudioNormalSoundAsset> NormalSoundAsset = nullptr;

	UPROPERTY(Transient)
	float CachedGlobalVolumeMultiplier = 1.0f;
	
	// base function
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySound2D(USoundBase* Sound, float VolumeMultiplier = 1.f, 
    	float PitchMultiplier = 1.f,float StartTime = 0.f);
    
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySoundAtLocation(USoundBase* Sound, FVector Location, 
    	FRotator Rotation = FRotator::ZeroRotator, float VolumeMultiplier = 1.f, 
    	float PitchMultiplier = 1.f, float StartTime = 0.f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySoundAttached(USoundBase* Sound, USceneComponent* AttachToComponent, 
    	FName AttachPointName = NAME_None, FVector Location = FVector::ZeroVector, 
    	EAttachLocation::Type LocationType = EAttachLocation::KeepRelativeOffset, 
    	bool bStopWhenAttachedToDestroyed = true, float VolumeMultiplier = 1.f, 
    	float PitchMultiplier = 1.f, float StartTime = 0.f);

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	//outer called function
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayNormalSound2D(FGameplayTag SoundTag);
	
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayNormalSoundAtLocation(FGameplayTag SoundTag, FVector Location);
	
};
