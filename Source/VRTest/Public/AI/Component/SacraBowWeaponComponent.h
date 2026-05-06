// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Component/SacraEnemyWeaponComponent.h"

#include "SacraBowWeaponComponent.generated.h"

class ACharacter;
class ASacraEnemyArrowProjectile;
class ASacraEnemyBowActor;
class UAnimInstance;
class UAnimMontage;
class USkeletalMeshComponent;

UCLASS(Blueprintable, BlueprintType, ClassGroup = AI, meta = (BlueprintSpawnableComponent))
class VRTEST_API USacraBowWeaponComponent : public USacraEnemyWeaponComponent
{
	GENERATED_BODY()

public:
	USacraBowWeaponComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual bool InitWeapon() override;
	virtual bool EquipWeapon() override;
	virtual void UnequipWeapon() override;
	virtual bool StartAttack(AActor* InTargetActor) override;
	virtual bool IsTargetInAttackRange(const AActor* TargetActor, float DistanceToTarget) const override;
	virtual bool CanAttackTarget(const AActor* TargetActor, float DistanceToTarget) const override;
	virtual bool ShouldKeepWeaponEquipped(float DistanceToTarget) const override;
	virtual bool IsInAttackRecovery() const override;

	UFUNCTION(BlueprintPure, Category = "AI|Weapon|Bow")
	bool IsWeaponAiming() const { return bIsWeaponAiming; }

	UFUNCTION(BlueprintPure, Category = "AI|Weapon|Bow")
	bool ShouldAimAtTarget(const AActor* TargetActor, float DistanceToTarget, bool bHasLineOfSight) const;

	UFUNCTION(BlueprintCallable, Category = "AI|Weapon|Bow")
	void SetWeaponAiming(bool bInAiming, AActor* InTargetActor = nullptr);

	UFUNCTION(BlueprintCallable, Category = "AI|Weapon|Bow")
	bool NotifyAttackRelease();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Bow")
	TSubclassOf<ASacraEnemyBowActor> BowActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Bow")
	TSubclassOf<ASacraEnemyArrowProjectile> ArrowProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Bow")
	FName BowEquipSocketName = TEXT("Archer_Bow_Equip");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Bow")
	FName ArrowNockSocketName = TEXT("Archer_Arrow");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Bow")
	FName PullStringSocketName = TEXT("Archer_Arrow");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Bow")
	TObjectPtr<UAnimMontage> EquipMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Bow")
	TObjectPtr<UAnimMontage> AttackMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Bow", meta = (ClampMin = "0.0"))
	float ArrowLaunchSpeed = 2200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Bow", meta = (ClampMin = "0.0"))
	float ProjectileGravityScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Bow", meta = (ClampMin = "0.0"))
	float MinAttackRange = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Bow", meta = (ClampMin = "0.0"))
	float MaxAttackRange = 1400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Bow", meta = (ClampMin = "0.0"))
	float UnequipBeyondRange = 1800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Bow", meta = (ClampMin = "0.0"))
	float AttackCooldownDuration = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Bow")
	bool bUseSuggestProjectileVelocity = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Bow")
	bool bLeadMovingTargets = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Bow", meta = (ClampMin = "0.0"))
	float MaxLeadPredictionTime = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Bow", meta = (ClampMin = "0.0"))
	float FallbackTargetAimHeightOffset = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Bow")
	bool bHideBowWhenUnequipped = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Bow")
	bool bDestroyLoadedArrowOnUnequip = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Bow")
	bool bKeepArrowLoadedWhenNotAiming = false;

private:
	bool SpawnBowIfNeeded();
	bool ReloadArrowIfNeeded();
	void AttachBowToOwner();
	void CleanupSpawnedActors();
	void SetWeaponAimingState(bool bInAiming);
	void SyncLoadedArrowToBow() const;
	void RefreshVisualState() const;
	virtual void HandleWeaponPausedStateChanged() override;

	UAnimInstance* GetOwnerAnimInstance() const;
	float PlayOwnerMontage(UAnimMontage* MontageToPlay) const;
	bool ReleaseCurrentArrowAtTarget(AActor* InTargetActor);
	FVector GetTargetAimLocation(AActor* InTargetActor) const;
	FVector GetPredictedTargetLocation(AActor* InTargetActor, const FVector& StartLocation) const;
	void UpdateBowVisualPullState(float PullAlpha);

	void HandleEquipMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void HandleAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void HandleAttackCooldownFinished();
	UFUNCTION()
	void HandleWeaponAttackFinishedInternal(bool bSuccess);

private:
	UPROPERTY(Transient)
	TObjectPtr<ACharacter> CachedOwnerCharacter = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USkeletalMeshComponent> CachedOwnerMesh = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<ASacraEnemyBowActor> SpawnedBowActor = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<ASacraEnemyArrowProjectile> LoadedArrowProjectile = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<AActor> PendingAttackTarget = nullptr;

	UPROPERTY(Transient)
	bool bIsAttackOnCooldown = false;

	UPROPERTY(Transient)
	bool bPendingStartAttackCooldown = false;

	UPROPERTY(Transient)
	bool bIsWeaponAiming = false;

	FTimerHandle AttackCooldownTimerHandle;

	UPROPERTY(Transient)
	bool bHasReceivedAttackReleaseNotify = false;
};
