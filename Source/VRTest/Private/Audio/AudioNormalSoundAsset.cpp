#include "Audio/AudioNormalSoundAsset.h"

#include "Sound/SoundBase.h"

USoundBase* UAudioNormalSoundAsset::GetNormalSound(FGameplayTag SoundTag) const
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
