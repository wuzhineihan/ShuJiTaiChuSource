// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Component/SacraEnemyLoadoutComponent.h"

#include "SkeletalMergingLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Effect/EffectTypes.h"
#include "GameFramework/Character.h"

USacraEnemyLoadoutComponent::USacraEnemyLoadoutComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USacraEnemyLoadoutComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoInitializeLoadoutOnBeginPlay)
	{
		InitializeLoadout();
	}
}

bool USacraEnemyLoadoutComponent::InitializeLoadout()
{
	if (bLoadoutInitialized)
	{
		return true;
	}

	if (!ResolveOwnerMesh())
	{
		return false;
	}

	if (LoadoutDataAsset)
	{
		if (const FSacraEnemyAppearancePreset* AppearancePreset = SelectAppearanceOption())
		{
			ApplyMergedAppearance(*AppearancePreset);
		}

		if (LoadoutDataAsset->MeshPhysicsAsset)
		{
			CachedOwnerMesh->SetPhysicsAsset(LoadoutDataAsset->MeshPhysicsAsset);
		}

		if (!LoadoutDataAsset->MeshCollisionProfileName.IsNone())
		{
			CachedOwnerMesh->SetCollisionProfileName(LoadoutDataAsset->MeshCollisionProfileName);
		}
	}

	bLoadoutInitialized = true;
	return true;
}

bool USacraEnemyLoadoutComponent::CanReceiveArrowDamage(const FEffect& Effect) const
{
	if (GetArmorType() != ESacraEnemyArmorType::Heavy)
	{
		return true;
	}

	return EffectContainsFire(Effect);
}

void USacraEnemyLoadoutComponent::ApplyConfigData(const FSacraEnemyLoadoutConfig& ConfigData)
{
	if (!ConfigData.bOverrideLoadoutConfig)
	{
		return;
	}

	bAutoInitializeLoadoutOnBeginPlay = ConfigData.bAutoInitializeLoadoutOnBeginPlay;
	LoadoutDataAsset = ConfigData.LoadoutDataAsset;
	bRandomizeAppearance = ConfigData.bRandomizeAppearance;
	AppearanceIndex = ConfigData.AppearanceIndex;
}

bool USacraEnemyLoadoutComponent::ResolveOwnerMesh()
{
	CachedOwnerCharacter = Cast<ACharacter>(GetOwner());
	CachedOwnerMesh = CachedOwnerCharacter ? CachedOwnerCharacter->GetMesh() : nullptr;
	return IsValid(CachedOwnerCharacter) && IsValid(CachedOwnerMesh);
}

const FSacraEnemyAppearancePreset* USacraEnemyLoadoutComponent::SelectAppearanceOption() const
{
	if (!LoadoutDataAsset || LoadoutDataAsset->AppearancePresets.IsEmpty())
	{
		return nullptr;
	}

	if (bRandomizeAppearance)
	{
		const int32 RandomIndex = FMath::RandRange(0, LoadoutDataAsset->AppearancePresets.Num() - 1);
		return &LoadoutDataAsset->AppearancePresets[RandomIndex];
	}

	const int32 ClampedIndex = FMath::Clamp(AppearanceIndex, 0, LoadoutDataAsset->AppearancePresets.Num() - 1);
	return &LoadoutDataAsset->AppearancePresets[ClampedIndex];
}

bool USacraEnemyLoadoutComponent::ApplyMergedAppearance(const FSacraEnemyAppearancePreset& AppearancePreset)
{
	if (!IsValid(CachedOwnerMesh))
	{
		return false;
	}

	if (AppearancePreset.MeshesToMerge.IsEmpty())
	{
		return false;
	}

	FSkeletalMeshMergeParams MergeParams;
	MergeParams.MeshesToMerge = AppearancePreset.MeshesToMerge;
	MergeParams.Skeleton = AppearancePreset.Skeleton;
	MergeParams.StripTopLODS = AppearancePreset.StripTopLODs;
	MergeParams.bNeedsCpuAccess = AppearancePreset.bNeedsCpuAccess;

	USkeletalMesh* MergedMesh = USkeletalMergingLibrary::MergeMeshes(MergeParams);
	if (!IsValid(MergedMesh))
	{
		return false;
	}

	CachedOwnerMesh->SetSkeletalMeshAsset(MergedMesh);
	return true;
}

bool USacraEnemyLoadoutComponent::EffectContainsFire(const FEffect& Effect) const
{
	return Effect.EffectTypes.Contains(EEffectType::Fire);
}
