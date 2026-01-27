#include "Audio/AudioSubsystem.h"

#include "Game/GameSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void UAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	NormalSoundAsset = nullptr;
	CachedGlobalVolumeMultiplier = 1.0f;

	if (const UGameSettings* Settings = UGameSettings::Get())
	{
		NormalSoundAsset = Settings->GetNormalSoundAsset();
		CachedGlobalVolumeMultiplier = Settings->GetGlobalVolumeMultiplier();
	}
}

void UAudioSubsystem::Deinitialize()
{
	NormalSoundAsset = nullptr;

	Super::Deinitialize();
}

void UAudioSubsystem::PlaySound2D(USoundBase* Sound, float VolumeMultiplier, float PitchMultiplier, float StartTime)
{
	if (!Sound || !GetWorld())
	{
		return;
	}

	UGameplayStatics::PlaySound2D(GetWorld(), Sound, VolumeMultiplier * CachedGlobalVolumeMultiplier, PitchMultiplier, StartTime);
}

void UAudioSubsystem::PlaySoundAtLocation(USoundBase* Sound, FVector Location, FRotator Rotation, float VolumeMultiplier, float PitchMultiplier, float StartTime)
{
	if (!Sound || !GetWorld())
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, Location, Rotation, VolumeMultiplier * CachedGlobalVolumeMultiplier, PitchMultiplier, StartTime);
}

void UAudioSubsystem::PlaySoundAttached(USoundBase* Sound, USceneComponent* AttachToComponent, FName AttachPointName,
	FVector Location, EAttachLocation::Type LocationType, bool bStopWhenAttachedToDestroyed, float VolumeMultiplier,
	float PitchMultiplier, float StartTime)
{
	if (!Sound || !GetWorld())
	{
		return;
	}

	UGameplayStatics::SpawnSoundAttached(Sound, AttachToComponent, AttachPointName, Location, LocationType, bStopWhenAttachedToDestroyed, VolumeMultiplier * CachedGlobalVolumeMultiplier, PitchMultiplier, StartTime);
}

void UAudioSubsystem::PlayNormalSound2D(FGameplayTag SoundTag)
{
	if (!NormalSoundAsset)
	{
		return;
	}

	USoundBase* Sound = NormalSoundAsset->GetNormalSound(SoundTag);
	if (!Sound)
	{
		UE_LOG(LogTemp, Warning, TEXT("UAudioSubsystem::PlayNormalSound2D: Failed to find sound for tag [%s]"), *SoundTag.ToString());
		return;
	}
	PlaySound2D(Sound);
}

void UAudioSubsystem::PlayNormalSoundAtLocation(FGameplayTag SoundTag, FVector Location)
{
	if (!NormalSoundAsset)
	{
		return;
	}

	USoundBase* Sound = NormalSoundAsset->GetNormalSound(SoundTag);
	if (!Sound)
	{
		UE_LOG(LogTemp, Warning, TEXT("UAudioSubsystem::PlayNormalSoundAtLocation: Failed to find sound for tag [%s]"), *SoundTag.ToString());
		return;
	}
	PlaySoundAtLocation(Sound, Location);
}
