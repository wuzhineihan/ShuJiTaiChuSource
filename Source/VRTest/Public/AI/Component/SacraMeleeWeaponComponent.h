// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Component/SacraEnemyWeaponComponent.h"

#include "SacraMeleeWeaponComponent.generated.h"

class ACharacter;
class UAnimInstance;
class UAnimMontage;
class USkeletalMeshComponent;

UCLASS(Blueprintable, BlueprintType, ClassGroup = AI, meta = (BlueprintSpawnableComponent))
class VRTEST_API USacraMeleeWeaponComponent : public USacraEnemyWeaponComponent
{
	GENERATED_BODY()

public:
	USacraMeleeWeaponComponent();

	virtual void BeginPlay() override;

	// ==================== Public Interface ====================

	virtual bool InitWeapon() override;
	virtual bool EquipWeapon() override;
	virtual void UnequipWeapon() override;
	virtual bool StartAttack(AActor* InTargetActor) override;
	virtual void HandleOwnerDeath() override;
	virtual bool IsTargetInAttackRange(const AActor* TargetActor, float DistanceToTarget) const override;
	virtual bool ShouldUseDirectApproachToTarget(const AActor* TargetActor, float DistanceToTarget) const override;

	// 由动画通知或外部流程显式调用，控制武器何时真正挂到角色身上。
	UFUNCTION(BlueprintCallable, Category = "AI|Weapon|Melee")
	void AttachWeaponToOwner();

	// 由动画通知或外部流程显式调用，控制武器何时从角色身上卸下。
	UFUNCTION(BlueprintCallable, Category = "AI|Weapon|Melee")
	void DetachWeaponFromOwner();

	// 由装备蒙太奇通知调用。这里只在真正挂武器后才把 Equipped 置为 true。
	UFUNCTION(BlueprintCallable, Category = "AI|Weapon|Melee")
	bool CompleteMeleeEquip();

	UFUNCTION(BlueprintCallable, Category = "AI|Weapon|Melee")
	bool BeginAttackWindow();

	UFUNCTION(BlueprintCallable, Category = "AI|Weapon|Melee")
	bool EndAttackWindow();

	UFUNCTION(BlueprintCallable, Category = "AI|Weapon|Melee")
	bool ApplyAttackHit();

protected:
	// ==================== Config ====================

	// 近战武器 Actor 类型，装备时按需生成实例。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Melee")
	TSubclassOf<AActor> WeaponActorClass;

	// 武器挂接到角色 Mesh 的目标 Socket。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Melee")
	FName AttachSocketName = TEXT("socket_weapon_r");

	// 装备时播放的蒙太奇，真正挂武器的时机交给动画通知或外部逻辑。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Melee")
	TObjectPtr<UAnimMontage> EquipMontage = nullptr;

	// 攻击时播放的蒙太奇，命中与结束都交给动画通知或外部逻辑。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Melee")
	TObjectPtr<UAnimMontage> AttackMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Melee")
	bool bUseAttackWindowNotify = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Melee", meta = (ClampMin = "0.0"))
	float AttackDamage = 20.0f;

	// 是否在初始化后立刻生成武器实例，方便提前准备挂点对象。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Melee")
	bool bCreateWeaponOnInit = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Melee", meta = (ClampMin = "0.0"))
	float MinAttackRange = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Weapon|Melee", meta = (ClampMin = "0.0"))
	float MaxAttackRange = 180.0f;

private:
	// ==================== Internal Helpers ====================

	bool SpawnWeaponIfNeeded();
	UAnimInstance* GetOwnerAnimInstance() const;
	float PlayOwnerMontage(UAnimMontage* MontageToPlay) const;
	void SyncRuntimeDurationsFromMontages();
	void HandleEquipMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void HandleAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	bool TryResolveAttackTarget(AActor*& OutTargetActor) const;

private:
	// ==================== Runtime ====================

	UPROPERTY(Transient)
	TObjectPtr<ACharacter> CachedOwnerCharacter = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USkeletalMeshComponent> CachedOwnerMesh = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<AActor> SpawnedWeaponActor = nullptr;

	UPROPERTY(Transient)
	bool bIsAttackWindowOpen = false;

	UPROPERTY(Transient)
	bool bHasAppliedAttackHit = false;
};
