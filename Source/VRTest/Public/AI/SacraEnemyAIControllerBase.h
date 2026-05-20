// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SacraEnemyAIControllerBase.generated.h"

class UAIPerceptionComponent;
class UBehaviorTree;
class UBehaviorTreeComponent;
class USacraBlackboardComponent;
class USacraEnemyConfigDataAsset;
class USacraEnemyHatredComponent;
class ACharacter;
class APawn;

/**
 * 
 */
UCLASS()
class VRTEST_API ASacraEnemyAIControllerBase : public AAIController
{
	GENERATED_BODY()
public:
	static UAIPerceptionComponent* FindPerceptionComponent(AActor* Actor);
	ASacraEnemyAIControllerBase();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION(BlueprintCallable, Category = "AI|Runtime")
	void SetEnemyAIPaused(bool bInPaused);

	UFUNCTION(BlueprintCallable, Category = "AI|Runtime")
	void SetEnemyRenderingEnabled(bool bInEnabled);

	UFUNCTION(BlueprintCallable, Category = "AI|Config")
	void ApplyConfigDataAsset(const USacraEnemyConfigDataAsset* ConfigDataAsset);

	UFUNCTION(BlueprintPure, Category = "AI|Runtime")
	bool IsEnemyAIPaused() const { return bIsEnemyAIPaused; }

private:
	void EnsureRuntimeComponents();
	void TryStartBehaviorTree();
	void ApplyPausedStateToControllerComponents();
	void ApplyPausedStateToPawnComponents();
	void ApplyPawnAnimationState();
	void ApplyBehaviorSubtreeConfig();
	void BindHatredDelegates();
	void UnbindHatredDelegates();
	void RefreshRotationMode();
	void ApplyMovementRotationMode(bool bFaceMovement) const;
	void ApplyFightFocus(AActor* FocusTarget) const;

	UFUNCTION()
	void HandleHatredStateChanged(EHatredState NewState);

	UFUNCTION()
	void HandleFightTargetChanged(AActor* NewTargetActor);

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|BehaviorTree", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBehaviorTree> DefaultBehaviorTreeAsset = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|BehaviorTree", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBehaviorTree> IdleBehaviorSubtreeAsset = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|BehaviorTree", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBehaviorTree> WarningBehaviorSubtreeAsset = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|BehaviorTree", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBehaviorTree> FightBehaviorSubtreeAsset = nullptr;

	UPROPERTY(Transient, VisibleInstanceOnly, Category = "AI|Runtime", meta = (AllowPrivateAccess = "true"))
	bool bIsEnemyAIPaused = false;

	UPROPERTY(Transient, VisibleInstanceOnly, Category = "AI|Runtime", meta = (AllowPrivateAccess = "true"))
	bool bIsEnemyRenderingEnabled = true;

	UPROPERTY(Transient)
	bool bCachedMeshAnimationState = false;

	UPROPERTY(Transient)
	bool bCachedMeshPauseAnims = false;

	UPROPERTY(Transient)
	bool bCachedMeshNoSkeletonUpdate = false;

	UPROPERTY(Transient)
	uint8 CachedVisibilityBasedAnimTickOption = 0;


	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "AI",meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USacraBlackboardComponent> EnemyBlackboardComponent;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "AI",meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USacraEnemyHatredComponent> EnemyHatredComponent;	

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Rotation", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float NonFightRotationRateYaw = 360.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Rotation", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float FightRotationRateYaw = 540.0f;
};
