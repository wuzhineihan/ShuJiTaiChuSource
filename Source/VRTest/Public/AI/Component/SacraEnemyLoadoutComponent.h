// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AI/DataAsset/SacraEnemyConfigDataAsset.h"
#include "AI/DataAsset/SacraEnemyLoadoutDataAsset.h"

#include "SacraEnemyLoadoutComponent.generated.h"

struct FEffect;
class ACharacter;
class USkeletalMeshComponent;

UCLASS(Blueprintable, BlueprintType, ClassGroup = AI, meta = (BlueprintSpawnableComponent))
class VRTEST_API USacraEnemyLoadoutComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USacraEnemyLoadoutComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "AI|Loadout")
	bool InitializeLoadout();

	UFUNCTION(BlueprintPure, Category = "AI|Loadout")
	bool IsLoadoutInitialized() const { return bLoadoutInitialized; }

	UFUNCTION(BlueprintPure, Category = "AI|Loadout")
	ESacraEnemyArmorType GetArmorType() const { return LoadoutDataAsset ? LoadoutDataAsset->ArmorType : ESacraEnemyArmorType::None; }

	UFUNCTION(BlueprintPure, Category = "AI|Loadout")
	bool IsHeavyArmor() const { return GetArmorType() == ESacraEnemyArmorType::Heavy; }

	UFUNCTION(BlueprintPure, Category = "AI|Loadout")
	bool CanReceiveArrowDamage(const FEffect& Effect) const;

	UFUNCTION(BlueprintCallable, Category = "AI|Loadout|Config")
	void ApplyConfigData(const FSacraEnemyLoadoutConfig& ConfigData);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Loadout")
	bool bAutoInitializeLoadoutOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Loadout")
	TObjectPtr<USacraEnemyLoadoutDataAsset> LoadoutDataAsset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Loadout")
	bool bRandomizeAppearance = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Loadout", meta = (ClampMin = "0"))
	int32 AppearanceIndex = 0;

private:
	bool ResolveOwnerMesh();
	const FSacraEnemyAppearancePreset* SelectAppearanceOption() const;
	bool ApplyMergedAppearance(const FSacraEnemyAppearancePreset& AppearancePreset);
	bool EffectContainsFire(const FEffect& Effect) const;

	UPROPERTY(Transient)
	TObjectPtr<ACharacter> CachedOwnerCharacter = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USkeletalMeshComponent> CachedOwnerMesh = nullptr;

	UPROPERTY(Transient)
	bool bLoadoutInitialized = false;
};
