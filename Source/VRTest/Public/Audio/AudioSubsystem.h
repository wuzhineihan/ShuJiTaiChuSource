#pragma once

#include "CoreMinimal.h"
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

public:
	/**
	 * 播放 2D 音效/音乐。
	 * @param WorldContextObject 传入任意能拿到 World 的对象（PlayerController/Actor/Component 等）。
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio", meta = (WorldContext = "WorldContextObject"))
	void PlaySound2D(const UObject* WorldContextObject, USoundBase* Sound, float VolumeMultiplier = 1.f, float PitchMultiplier = 1.f, float StartTime = 0.f);

	/**
	 * 在世界坐标播放一次性音效。
	 * @param WorldContextObject 传入任意能拿到 World 的对象（PlayerController/Actor/Component 等）。
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio", meta = (WorldContext = "WorldContextObject"))
	void PlaySoundAtLocation(const UObject* WorldContextObject, USoundBase* Sound, FVector Location, FRotator Rotation = FRotator::ZeroRotator, float VolumeMultiplier = 1.f, float PitchMultiplier = 1.f, float StartTime = 0.f);
};
