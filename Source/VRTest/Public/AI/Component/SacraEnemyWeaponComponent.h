// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "SacraEnemyWeaponComponent.generated.h"

class AActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyWeaponEquippedChanged, bool, bIsEquipped);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyWeaponAttackStarted, AActor*, TargetActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyWeaponAttackFinished, bool, bSuccess);

USTRUCT(BlueprintType)
struct FEnemyWeaponEquipFinishedMessage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Weapon")
	TObjectPtr<USacraEnemyWeaponComponent> WeaponComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Weapon")
	TObjectPtr<AActor> InstigatorActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Weapon")
	bool bSuccess = true;
};

USTRUCT(BlueprintType)
struct FEnemyWeaponAttackFinishedMessage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Weapon")
	TObjectPtr<USacraEnemyWeaponComponent> WeaponComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Weapon")
	TObjectPtr<AActor> InstigatorActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Weapon")
	bool bSuccess = true;
};

UCLASS(Blueprintable, BlueprintType, ClassGroup = AI, meta = (BlueprintSpawnableComponent))
class VRTEST_API USacraEnemyWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USacraEnemyWeaponComponent();

	virtual void BeginPlay() override;

	// ==================== Public Interface ====================

	UFUNCTION(BlueprintCallable, Category = "AI|Weapon")
	virtual bool InitWeapon();

	UFUNCTION(BlueprintCallable, Category = "AI|Weapon")
	void SetWeaponPaused(bool bInPaused);

	UFUNCTION(BlueprintCallable, Category = "AI|Weapon")
	virtual bool EquipWeapon();

	// Marks the weapon as fully equipped after any deferred equip flow finishes and broadcasts the equip-finished message.
	UFUNCTION(BlueprintCallable, Category = "AI|Weapon")
	bool CompleteEquipWeapon();

	UFUNCTION(BlueprintCallable, Category = "AI|Weapon")
	virtual void UnequipWeapon();

	UFUNCTION(BlueprintCallable, Category = "AI|Weapon")
	virtual bool StartAttack(AActor* InTargetActor);

	// Must be called by anim notify or external combat flow when the current attack actually ends.
	UFUNCTION(BlueprintCallable, Category = "AI|Weapon")
	void FinishAttack(bool bSuccess = true);

	UFUNCTION(BlueprintCallable, Category = "AI|Weapon")
	virtual void HandleOwnerDeath();

	UFUNCTION(BlueprintPure, Category = "AI|Weapon")
	bool IsWeaponInitialized() const { return bIsWeaponInitialized; }

	UFUNCTION(BlueprintPure, Category = "AI|Weapon")
	bool IsWeaponEquipped() const { return bIsWeaponEquipped; }

	UFUNCTION(BlueprintPure, Category = "AI|Weapon")
	bool IsAttacking() const { return bIsAttacking; }

	UFUNCTION(BlueprintPure, Category = "AI|Weapon")
	bool IsWeaponPaused() const { return bIsWeaponPaused; }

	UFUNCTION(BlueprintPure, Category = "AI|Weapon")
	AActor* GetCurrentAttackTarget() const { return CurrentAttackTarget.Get(); }

	UFUNCTION(BlueprintPure, Category = "AI|Weapon")
	float GetEquipDuration() const { return EquipDuration; }

	UFUNCTION(BlueprintPure, Category = "AI|Weapon")
	float GetAttackDuration() const { return AttackDuration; }

	UFUNCTION(BlueprintPure, Category = "AI|Weapon")
	virtual bool IsTargetInAttackRange(const AActor* TargetActor, float DistanceToTarget) const;

	UFUNCTION(BlueprintPure, Category = "AI|Weapon")
	virtual bool CanAttackTarget(const AActor* TargetActor, float DistanceToTarget) const;

	UFUNCTION(BlueprintPure, Category = "AI|Weapon")
	virtual bool ShouldKeepWeaponEquipped(float DistanceToTarget) const;

	UFUNCTION(BlueprintPure, Category = "AI|Weapon")
	virtual bool IsInAttackRecovery() const { return false; }

	UFUNCTION(BlueprintPure, Category = "AI|Weapon")
	virtual bool ShouldUseDirectApproachToTarget(const AActor* TargetActor, float DistanceToTarget) const { return false; }

	// ==================== Events ====================

	UPROPERTY(BlueprintAssignable, Category = "AI|Weapon|Event")
	FOnEnemyWeaponEquippedChanged OnWeaponEquippedChanged;

	UPROPERTY(BlueprintAssignable, Category = "AI|Weapon|Event")
	FOnEnemyWeaponAttackStarted OnWeaponAttackStarted;

	UPROPERTY(BlueprintAssignable, Category = "AI|Weapon|Event")
	FOnEnemyWeaponAttackFinished OnWeaponAttackFinished;

protected:
	// ==================== Config ====================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Config", meta = (ClampMin = "0.0"))
	float EquipDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Config", meta = (ClampMin = "0.0"))
	float AttackDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Config")
	bool bAutoInitOnBeginPlay = false;

	// ==================== Internal Helpers ====================

	void SetWeaponInitialized(bool bInInitialized);

	void SetWeaponEquippedState(bool bInEquipped);
	void BroadcastEquipFinishedMessage(bool bSuccess);
	virtual void HandleWeaponPausedStateChanged();

private:
	// ==================== Runtime ====================

	UPROPERTY(Transient)
	bool bIsWeaponInitialized = false;

	UPROPERTY(Transient)
	bool bIsWeaponEquipped = false;

	UPROPERTY(Transient)
	bool bIsAttacking = false;

	UPROPERTY(Transient)
	bool bIsWeaponPaused = false;

	UPROPERTY(Transient)
	TObjectPtr<AActor> CurrentAttackTarget = nullptr;
};
