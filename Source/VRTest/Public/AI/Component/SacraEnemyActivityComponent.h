// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "SacraEnemyActivityComponent.generated.h"

class AActor;
class ALevelSequenceActor;
class ASacraEnemyAIControllerBase;
class UCharacterMovementComponent;
class ULevelSequencePlayer;
class USacraEnemyHatredComponent;
class USacraEnemyWeaponComponent;

UENUM(BlueprintType)
enum class ESacraEnemySpecialActivityType : uint8
{
	None,
	Sequence,
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSacraEnemySpecialActivityChanged, ESacraEnemySpecialActivityType, ActivityType, bool, bIsPlaying);

UCLASS(ClassGroup = AI, meta = (BlueprintSpawnableComponent))
class VRTEST_API USacraEnemyActivityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USacraEnemyActivityComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "AI|Activity")
	bool StartSequenceActivity(ALevelSequenceActor* SequenceActor);

	UFUNCTION(BlueprintCallable, Category = "AI|Activity")
	void StopCurrentActivity(bool bRestoreState = false);

	UFUNCTION(BlueprintPure, Category = "AI|Activity")
	bool IsSpecialActivityActive() const { return CurrentActivityType != ESacraEnemySpecialActivityType::None; }

	UFUNCTION(BlueprintPure, Category = "AI|Activity")
	bool IsSpecialActivityPlaying() const;

	UFUNCTION(BlueprintPure, Category = "AI|Activity")
	ESacraEnemySpecialActivityType GetCurrentActivityType() const { return CurrentActivityType; }

	UFUNCTION(BlueprintPure, Category = "AI|Activity")
	ALevelSequenceActor* GetCurrentSequenceActor() const { return CurrentSequenceActor.Get(); }

	UPROPERTY(BlueprintAssignable, Category = "AI|Activity|Event")
	FOnSacraEnemySpecialActivityChanged OnSpecialActivityChanged;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Activity|Sequence")
	bool bInterruptSequenceOnFight = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Activity|Sequence")
	bool bPauseWeaponWhileSequence = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Activity|Sequence")
	bool bDisableMovementWhileSequence = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Activity|Sequence")
	bool bDisableCameraCutsWhileSequence = true;

private:
	void ResolveRuntimeReferences();
	void BindHatredDelegate();
	void UnbindHatredDelegate();
	void BindSequencePlayerDelegate();
	void UnbindSequencePlayerDelegate();
	void BroadcastActivityStateChanged();
	void ClearCurrentActivityState();
	void ApplySequenceRuntimeLocks();
	void ReleaseSequenceRuntimeLocks();

	UFUNCTION()
	void HandleHatredStateChanged(EHatredState NewState);

	UFUNCTION()
	void HandleSequenceFinished();

private:
	UPROPERTY(Transient)
	TObjectPtr<ASacraEnemyAIControllerBase> CachedEnemyController = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USacraEnemyHatredComponent> CachedHatredComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USacraEnemyWeaponComponent> CachedWeaponComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UCharacterMovementComponent> CachedMovementComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<ALevelSequenceActor> CurrentSequenceActor = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<ULevelSequencePlayer> CurrentSequencePlayer = nullptr;

	UPROPERTY(Transient)
	ESacraEnemySpecialActivityType CurrentActivityType = ESacraEnemySpecialActivityType::None;

	UPROPERTY(Transient)
	bool bAppliedMovementLock = false;

	UPROPERTY(Transient)
	bool bAppliedWeaponPause = false;
};
