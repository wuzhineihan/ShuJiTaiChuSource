# Audio Component Design (Extension of AudioComponent)

## 目录
- [Audio Component Design (Extension of AudioComponent)](#audio-component-design-extension-of-audiocomponent)
  - [目录](#目录)
  - [1. 核心目标](#1-核心目标)
  - [2. 核心设计决策](#2-核心设计决策)
    - [A. 继承自 UAudioComponent](#a-继承自-uaudiocomponent)
    - [B. 随机音色系统 (Voice Variation)](#b-随机音色系统-voice-variation)
    - [C. Tag 驱动的播放逻辑](#c-tag-驱动的播放逻辑)
  - [3. 数据结构定义](#3-数据结构定义)
  - [4. 接口定义 (C++ API)](#4-接口定义-c-api)
  - [5. 目录结构](#5-目录结构)
  - [6. API 速查 (External API Summary)](#6-api-速查-external-api-summary)
    - [Commands](#commands)
  - [7. 使用示例 (Example)](#7-使用示例-example)
    - [场景: 敌人攻击喊叫](#场景-敌人攻击喊叫)

## 1. 核心目标
构建一个具备 **语义化播放** 和 **音色随机化** 能力的音频组件。
- **语义化**：通过 `GameplayTag`（如 `Audio.Combat.Hurt`）请求播放，无需知道具体资源路径。
- **多样性**：支持为同一类角色配置多套“音色表”（Voice Profiles），生成时随机选取一套，使不同个体具有不同的声音特征（如低沉、尖锐、沙哑）。
- **空间化**：直接继承自 `UAudioComponent`，天然具备 3D 空间发声能力和挂载特性。

## 2. 核心设计决策

### A. 继承自 UAudioComponent
- 该组件直接是一个 `USceneComponent` 的子类（`UAudioComponent`），意味着它有具体的 3D 位置。
- **单通道特性**：作为一个组件实例，它同一时刻主要维护一个 `SoundBase` 的播放。这非常适合处理 **语音（Vocalization）** 或 **主要动作音效**（如脚步、挥动），如果需要新的声音打断旧的声音（如受击打断攻击喊叫），此架构非常自然。

### B. 随机音色系统 (Voice Variation)
- 引入 **音色配置 (Voice Profile)** 概念。
- 在 `BeginPlay` 阶段，组件从配置资产中随机选定一个 Profile，并将其缓存。
- 在该 Actor 的整个生命周期内，所有通过 Tag 请求的音效都将从这个选定的 Profile 中查找，保证了声音性格的一致性。

### C. Tag 驱动的播放逻辑
- 外部调用者只需要传递意图（Payload 中的 Tag）。
- 组件内部负责：`Tag -> Profile -> SoundBase` 的查找过程。
- 组件内部负责：`SetSound` -> `Play` 的底层操作。

## 3. 数据结构定义

```cpp
// 单个音色配置
USTRUCT(BlueprintType)
struct FVoiceProfile
{
    GENERATED_BODY()

public:
    // 该音色的描述（Debug用，如 "Deep Voice"）
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName ProfileName;

    // 标签到资源的映射表
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "Audio"))
    TMap<FGameplayTag, USoundBase*> TagToSoundMap;
};

// 全局/角色配置资产
UCLASS(BlueprintType)
class UEnemyAudioConfigDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    // 可选的音色列表
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FVoiceProfile> AvailableProfiles;
};

// 播放请求载荷
UCLASS(BlueprintType)
class UAudioRequestPayload : public UObject
{
    GENERATED_BODY()

public:
    // [必填] 音效意图标签
    UPROPERTY(BlueprintReadWrite, Category = "Request", meta = (ExposeOnSpawn = true))
    FGameplayTag AudioTag;

    // [可选] 音量倍率 (默认 1.0)
    UPROPERTY(BlueprintReadWrite, Category = "Request")
    float VolumeMultiplier = 1.0f;

    // [可选] 音高倍率 (默认 1.0)
    UPROPERTY(BlueprintReadWrite, Category = "Request")
    float PitchMultiplier = 1.0f;
    
    // [可选] 是否覆盖当前正在播放的声音 (默认 true)
    UPROPERTY(BlueprintReadWrite, Category = "Request")
    bool bStopCurrent = true;
};
```

## 4. 接口定义 (C++ API)

```cpp
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UGameAudioComponent : public UAudioComponent
{
    GENERATED_BODY()

public:
    // --- Configuration ---

    // 可以在蓝图中配置该角色的音效资产集合
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Audio")
    UEnemyAudioConfigDataAsset* AudioConfig;

    virtual void BeginPlay() override;

    // --- Commands ---

    /**
     * 播放指定标签的音效
     * 根据 BeginPlay 时随机选中的 Profile 查找对应的 SoundBase 并播放
     * @param Payload 包含 Tag 和播放参数
     * @return true 如果找到了资源并开始播放
     */
    UFUNCTION(BlueprintCallable, Category = "AI|Audio")
    bool PlayAudioByTag(UAudioRequestPayload* Payload);

    /**
     * 停止当前播放
     * 封装了 UAudioComponent::Stop()
     */
    UFUNCTION(BlueprintCallable, Category = "AI|Audio")
    void StopCurrentAudio();

protected:
    // --- Internal State ---

    // 当前选中的音色索引
    int32 CurrentProfileIndex = -1;
    
    // 缓存当前使用的 Map 指针，避免每次去 DataAsset 查找
    const TMap<FGameplayTag, USoundBase*>* CurrentSoundMap = nullptr;
};
```

## 5. 目录结构
`Source/VRTest/Public/AI/Component/GameAudioComponent.h`
`Source/VRTest/Private/AI/Component/GameAudioComponent.cpp`
`Source/VRTest/Public/AI/Data/EnemyAudioConfigDataAsset.h`

## 6. API 速查 (External API Summary)

### Commands
| 函数名 | 说明 | 适用场景 |
| :--- | :--- | :--- |
| **`PlayAudioByTag(Payload)`** | 根据内部选定的音色播放 Tag 对应的 Sound。 | 喊叫、对话、主要反馈音效。 |
| **`StopCurrentAudio()`** | 停止当前组件的声音。 | 死亡、被打断、状态重置。 |

## 7. 使用示例 (Example)

### 场景: 敌人攻击喊叫

```cpp
// 1. 初始化 (在 BeginPlay 自动完成随机音色选择)

// 2. 发起播放
void UEnemyCombatComponent::PerformAttack()
{
    // ... 攻击逻辑 ...
    
    // 获取组件
    auto* AudioComp = GetOwner()->FindComponentByClass<UGameAudioComponent>();
    if (AudioComp)
    {
        // 构建请求
        UAudioRequestPayload* Payload = NewObject<UAudioRequestPayload>();
        Payload->AudioTag = FGameplayTag::RequestGameplayTag("Audio.Voice.Attack");
        Payload->PitchMultiplier = FMath::RandRange(0.9f, 1.1f); // 哪怕是同一个音色，也加点微小随机
        
        // 播放
        AudioComp->PlayAudioByTag(Payload);
    }
}

// 3. 被打断/死亡
void UEnemyHealthComponent::OnDeath()
{
    auto* AudioComp = GetOwner()->FindComponentByClass<UGameAudioComponent>();
    if (AudioComp)
    {
        // 先停掉之前的攻击喊叫
        AudioComp->StopCurrentAudio();
        
        // 播放死亡音效
        UAudioRequestPayload* DeathPayload = NewObject<UAudioRequestPayload>();
        DeathPayload->AudioTag = FGameplayTag::RequestGameplayTag("Audio.Voice.Death");
        AudioComp->PlayAudioByTag(DeathPayload);
    }
}
```
