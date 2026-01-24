#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AudioNormalSoundSet.generated.h"

class USoundBase;

UCLASS(BlueprintType)
class VRTEST_API UAudioNormalSoundSet : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Data", meta = (Categories = "NormalSound"))
	TMap<FGameplayTag, TObjectPtr<USoundBase>> NormalSounds;


	UFUNCTION(BlueprintPure, Category = "Audio|Data")
	USoundBase* GetNormalSound(FGameplayTag SoundTag) const;
};

