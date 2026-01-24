#include "Audio/AudioSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void UAudioSubsystem::PlaySound2D(const UObject* WorldContextObject, USoundBase* Sound, float VolumeMultiplier, float PitchMultiplier, float StartTime)
{
	if (!WorldContextObject || !Sound)
	{
		return;
	}

	UGameplayStatics::PlaySound2D(WorldContextObject, Sound, VolumeMultiplier, PitchMultiplier, StartTime);
}

void UAudioSubsystem::PlaySoundAtLocation(const UObject* WorldContextObject, USoundBase* Sound, FVector Location, FRotator Rotation, float VolumeMultiplier, float PitchMultiplier, float StartTime)
{
	if (!WorldContextObject || !Sound)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(WorldContextObject, Sound, Location, Rotation, VolumeMultiplier, PitchMultiplier, StartTime);
}

