#include "Audio/AudioNormalSoundSet.h"

#include "Sound/SoundBase.h"

USoundBase* UAudioNormalSoundSet::GetNormalSound(FGameplayTag SoundTag) const
{
	if (!SoundTag.IsValid())
	{
		return nullptr;
	}

	if (const TObjectPtr<USoundBase>* Found = NormalSounds.Find(SoundTag))
	{
		return Found->Get();
	}

	return nullptr;
}

