// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AI/DataAsset/SacraEnemyConfigDataAsset.h"
#include "GameplayTagContainer.h"
#include "Perception/AIPerceptionTypes.h"
#include "TimerManager.h"
#include "GameFramework/GameplayMessageSubsystem.h"

#include "SacraEnemyHatredComponent.generated.h"

class AActor;
class ASacraEnemyAIControllerBase;
class ABaseEnemy;
class USacraEnemyHatredDataAsset;

UENUM(BlueprintType)
enum class EHatredState : uint8
{
	Idle,
	Warning,
	Fight,
};

USTRUCT(BlueprintType)
struct FEnemyHatredStateMessage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hatred")
	TObjectPtr<AActor> InstigatorActor = nullptr;
};

USTRUCT(BlueprintType)
struct FEnemyWarningAlertMessage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hatred")
	TObjectPtr<AActor> InstigatorActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hatred")
	FVector AlertLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hatred")
	TObjectPtr<AActor> FightTargetActor = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHatredStateChanged, EHatredState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHatredValueChanged, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFightTargetChanged, AActor*, NewTargetActor);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class VRTEST_API USacraEnemyHatredComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USacraEnemyHatredComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// ==================== Public Interface ====================

	void InitHatredComponent();
	void ResetHatredComponent();

	UFUNCTION(BlueprintCallable, Category = "Hatred")
	void SetHatredPaused(bool bInPaused);

	UFUNCTION(BlueprintPure, Category = "Hatred")
	bool IsHatredPaused() const { return bIsHatredPaused; }

	UFUNCTION(BlueprintCallable, Category = "Hatred")
	EHatredState GetCurrentHatredState() const { return CurrentHatredState; }

	UFUNCTION(BlueprintCallable, Category = "Hatred")
	float GetCurrentHatredValue() const { return CurrentHatredValue; }

	UFUNCTION(BlueprintCallable, Category = "Hatred")
	float GetMaxHatredValue() const;

	UFUNCTION(BlueprintCallable, Category = "Hatred")
	bool HasWarningTargetLocation() const { return bHasWarningTargetLocation; }

	UFUNCTION(BlueprintCallable, Category = "Hatred")
	FVector GetCurrentWarningTargetLocation() const { return CurrentWarningTargetLocation; }

	UFUNCTION(BlueprintCallable, Category = "Hatred")
	AActor* GetCurrentFightTargetActor() const { return CurrentFightTargetActor.Get(); }

	UFUNCTION(BlueprintCallable, Category = "Hatred")
	bool ApplyExternalWarningAlert(const FEnemyWarningAlertMessage& AlertMessage);

	UFUNCTION(BlueprintCallable, Category = "Hatred|Config")
	void ApplyConfigData(const FSacraEnemyHatredConfig& ConfigData);

	UFUNCTION()
	void OnPerceptionInfoUpdated(const FActorPerceptionUpdateInfo& UpdateInfo);

	UFUNCTION()
	void UpdateHatredValue();

	void OnWarningStateExitMessage(FGameplayTag Channel, const FEnemyHatredStateMessage& Message);

	// ==================== Events ====================

	UPROPERTY(BlueprintAssignable, Category = "Hatred|Event")
	FOnHatredStateChanged OnHatredStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Hatred|Event")
	FOnHatredValueChanged OnHatredValueChanged;

	UPROPERTY(BlueprintAssignable, Category = "Hatred|Event")
	FOnFightTargetChanged OnFightTargetChanged;

protected:
	// ==================== Config ====================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hatred|Config")
	TObjectPtr<USacraEnemyHatredDataAsset> HatredConfigAsset = nullptr;

private:
	// ==================== Internal Helpers ====================

	float GetConfiguredUpdateHatredValueInterval() const;
	float GetConfiguredWarningStateDecayInterval() const;
	float GetConfiguredFightStateDecayInterval() const;
	float GetConfiguredSightLoseGraceInterval() const;
	float GetConfiguredIdleSightGrowthBase() const;
	float GetConfiguredWarningSightGrowthBase() const;
	float GetConfiguredDefaultDecreaseHatredValueBase() const;
	float GetConfiguredMaxHatredValue() const;
	float GetConfiguredWarningStateThreshold() const;

	void BindPerceptionDelegates();
	void UnbindPerceptionDelegates();

	void RegisterGameplayMessageListener();
	void UnregisterGameplayMessageListener();

	void ChangeHatredState(EHatredState NewState);
	void ApplyHatredDelta(float DeltaValue);

	void ProcessIdleStateSense(float& OutDeltaValue);
	void ProcessWarningStateSense(float& OutDeltaValue);
	void ProcessFightStateSense(float& OutDeltaValue);

	void RefreshPerceptionFlags();
	bool HasAnyPerception() const;
	void SyncSightStimuliFromPerception();
	bool HasSuccessfulSightStimulusFromPerception(AActor* TargetActor, FAIStimulus* OutStimulus = nullptr) const;

	void CachePerceptionStimulus(TMap<TWeakObjectPtr<AActor>, FAIStimulus>& StimulusMap, AActor* TargetActor, const FAIStimulus& Stimulus);
	void RemovePerceptionStimulus(TMap<TWeakObjectPtr<AActor>, FAIStimulus>& StimulusMap, AActor* TargetActor);
	void CleanupStimulusMap(TMap<TWeakObjectPtr<AActor>, FAIStimulus>& StimulusMap) const;
	void ConsumeTransientPerceptionStimuli();
	void StartSightLoseGraceTimer(AActor* TargetActor);
	void CancelSightLoseGraceTimer(AActor* TargetActor);
	void ClearSightLoseGraceTimers();
	void HandleSightLoseGraceTimerExpired(TWeakObjectPtr<AActor> TargetActor);

	void UpdateTargetLocation(const FVector& InLocation);
	void UpdateTargetActor(AActor* InActor);
	bool TryApplyCorpseSightWarning(AActor* TargetActor, const FAIStimulus& Stimulus);
	void SetFightTargetActor(AActor* InActor);
	void UpdateFightTargetFromSight();
	bool TryGetNearestStimulusLocation(const TMap<TWeakObjectPtr<AActor>, FAIStimulus>& StimulusMap, FVector& OutLocation) const;

	void ClearWarningTargetLocation();
	void ClearFightTargetActor();
	void RememberLastKnownFightTargetLocation(const FVector& InLocation);
	void CacheLastKnownLocationFromFightTarget();

	void ClearHatredDecayTimers();
	void StartHatredValueUpdateTimer();
	void UpdateWarningStateDecayTimer();
	void UpdateFightStateDecayTimer();

	UFUNCTION()
	void HandleWarningStateDecayTimerExpired();

	UFUNCTION()
	void HandleFightStateDecayTimerExpired();

private:
	// ==================== Runtime State ====================

	UPROPERTY(BlueprintReadOnly, Category = "Hatred|Runtime", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ASacraEnemyAIControllerBase> CachedEnemyAIController;

	UPROPERTY(BlueprintReadOnly, Category = "Hatred|Runtime", meta = (AllowPrivateAccess = "true"))
	bool bIsHatredInitialized = false;

	UPROPERTY(BlueprintReadOnly, Category = "Hatred|Runtime", meta = (AllowPrivateAccess = "true"))
	bool bIsHatredPaused = false;

	FTimerHandle UpdateHatredValueTimerHandle;
	FTimerHandle WarningStateDecayTimerHandle;
	FTimerHandle FightStateDecayTimerHandle;

	FGameplayMessageListenerHandle WarningStateExitMessageHandle;

	UPROPERTY(BlueprintReadOnly, Category = "Hatred|Runtime", meta = (AllowPrivateAccess = "true"))
	EHatredState CurrentHatredState = EHatredState::Idle;

	UPROPERTY(BlueprintReadOnly, Category = "Hatred|Runtime", meta = (AllowPrivateAccess = "true"))
	float CurrentHatredValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Hatred|Runtime", meta = (AllowPrivateAccess = "true"))
	bool bHasWarningTargetLocation = false;

	UPROPERTY(BlueprintReadOnly, Category = "Hatred|Runtime", meta = (AllowPrivateAccess = "true"))
	FVector CurrentWarningTargetLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Hatred|Runtime", meta = (AllowPrivateAccess = "true"))
	bool bHasLastKnownFightTargetLocation = false;

	UPROPERTY(BlueprintReadOnly, Category = "Hatred|Runtime", meta = (AllowPrivateAccess = "true"))
	FVector LastKnownFightTargetLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Hatred|Runtime", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AActor> CurrentFightTargetActor = nullptr;

	TMap<TWeakObjectPtr<AActor>, FAIStimulus> CachedSightHatredTargetMap;
	TMap<TWeakObjectPtr<AActor>, FAIStimulus> CachedHearingHatredTargetMap;
	TMap<TWeakObjectPtr<AActor>, FAIStimulus> CachedDamageHatredTargetMap;
	TMap<TWeakObjectPtr<AActor>, FTimerHandle> PendingSightLoseTimerHandleMap;

	UPROPERTY(BlueprintReadOnly, Category = "Hatred|Runtime", meta = (AllowPrivateAccess = "true"))
	bool bIsSightPerceived = false;

	UPROPERTY(BlueprintReadOnly, Category = "Hatred|Runtime", meta = (AllowPrivateAccess = "true"))
	bool bIsHearingPerceived = false;

	UPROPERTY(BlueprintReadOnly, Category = "Hatred|Runtime", meta = (AllowPrivateAccess = "true"))
	bool bIsDamagePerceived = false;
};
