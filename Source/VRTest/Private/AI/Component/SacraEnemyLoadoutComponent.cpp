// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Component/SacraEnemyLoadoutComponent.h"

#include "SkeletalMergingLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Effect/EffectTypes.h"
#include "GameFramework/Character.h"
#include "UObject/Package.h"

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

	CaptureSourceMeshIfNeeded();

	if (LoadoutDataAsset)
	{
		if (IsValid(BakedMergedMesh))
		{
			CachedOwnerMesh->SetSkeletalMeshAsset(BakedMergedMesh);
		}
		else if (const FSacraEnemyAppearancePreset* AppearancePreset = SelectAppearanceOption(false))
		{
			ApplyMergedAppearance(*AppearancePreset, false);
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
	else
	{
		RestoreSourceMeshIfPossible();
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

	const bool bLoadoutSelectionChanged =
		LoadoutDataAsset != ConfigData.LoadoutDataAsset
		|| bRandomizeAppearance != ConfigData.bRandomizeAppearance
		|| AppearanceIndex != ConfigData.AppearanceIndex;

	bAutoInitializeLoadoutOnBeginPlay = ConfigData.bAutoInitializeLoadoutOnBeginPlay;
	LoadoutDataAsset = ConfigData.LoadoutDataAsset;
	bRandomizeAppearance = ConfigData.bRandomizeAppearance;
	AppearanceIndex = ConfigData.AppearanceIndex;
	ResetLoadoutState(true, bLoadoutSelectionChanged);
}

void USacraEnemyLoadoutComponent::ResetLoadoutState(bool bClearBakedMesh, bool bClearResolvedAppearance)
{
	bLoadoutInitialized = false;

	if (bClearBakedMesh)
	{
		BakedMergedMesh = nullptr;
	}

	if (bClearResolvedAppearance)
	{
		ResolvedAppearanceId = NAME_None;
		ResolvedAppearanceIndex = INDEX_NONE;
	}
}

bool USacraEnemyLoadoutComponent::ResolveOwnerMesh()
{
	CachedOwnerCharacter = Cast<ACharacter>(GetOwner());
	CachedOwnerMesh = CachedOwnerCharacter ? CachedOwnerCharacter->GetMesh() : nullptr;
	return IsValid(CachedOwnerCharacter) && IsValid(CachedOwnerMesh);
}

void USacraEnemyLoadoutComponent::CaptureSourceMeshIfNeeded()
{
	if (!IsValid(CachedOwnerMesh) || IsValid(SourceMeshAsset))
	{
		return;
	}

	SourceMeshAsset = CachedOwnerMesh->GetSkeletalMeshAsset();
}

void USacraEnemyLoadoutComponent::RestoreSourceMeshIfPossible() const
{
	if (!IsValid(CachedOwnerMesh) || !IsValid(SourceMeshAsset))
	{
		return;
	}

	CachedOwnerMesh->SetSkeletalMeshAsset(SourceMeshAsset);
}

const FSacraEnemyAppearancePreset* USacraEnemyLoadoutComponent::FindAppearanceOptionById(FName AppearanceId) const
{
	if (!LoadoutDataAsset || AppearanceId.IsNone())
	{
		return nullptr;
	}

	for (const FSacraEnemyAppearancePreset& AppearancePreset : LoadoutDataAsset->AppearancePresets)
	{
		if (AppearancePreset.AppearanceId == AppearanceId)
		{
			return &AppearancePreset;
		}
	}

	return nullptr;
}

const FSacraEnemyAppearancePreset* USacraEnemyLoadoutComponent::SelectAppearanceOption(bool bForceReselectRandom)
{
	if (!LoadoutDataAsset || LoadoutDataAsset->AppearancePresets.IsEmpty())
	{
		return nullptr;
	}

	if (bRandomizeAppearance)
	{
		if (!bForceReselectRandom)
		{
			if (const FSacraEnemyAppearancePreset* ExistingAppearance = FindAppearanceOptionById(ResolvedAppearanceId))
			{
				return ExistingAppearance;
			}

			if (LoadoutDataAsset->AppearancePresets.IsValidIndex(ResolvedAppearanceIndex))
			{
				return &LoadoutDataAsset->AppearancePresets[ResolvedAppearanceIndex];
			}
		}

		const int32 RandomIndex = FMath::RandRange(0, LoadoutDataAsset->AppearancePresets.Num() - 1);
		const FSacraEnemyAppearancePreset* SelectedAppearance = &LoadoutDataAsset->AppearancePresets[RandomIndex];
		ResolvedAppearanceId = SelectedAppearance->AppearanceId;
		ResolvedAppearanceIndex = RandomIndex;
		return SelectedAppearance;
	}

	const int32 ClampedIndex = FMath::Clamp(AppearanceIndex, 0, LoadoutDataAsset->AppearancePresets.Num() - 1);
	const FSacraEnemyAppearancePreset* SelectedAppearance = &LoadoutDataAsset->AppearancePresets[ClampedIndex];
	ResolvedAppearanceId = SelectedAppearance->AppearanceId;
	ResolvedAppearanceIndex = ClampedIndex;
	return SelectedAppearance;
}

bool USacraEnemyLoadoutComponent::ApplyMergedAppearance(const FSacraEnemyAppearancePreset& AppearancePreset, bool bPersistMergedMesh)
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

	USkeletalMesh* MeshToApply = MergedMesh;

#if WITH_EDITOR
	if (bPersistMergedMesh)
	{
		const FName BakedMeshName = MakeUniqueObjectName(this, USkeletalMesh::StaticClass(), TEXT("BakedEnemyLoadoutMesh"));
		MeshToApply = DuplicateObject<USkeletalMesh>(MergedMesh, this, BakedMeshName);
		if (!IsValid(MeshToApply))
		{
			return false;
		}

		MeshToApply->SetFlags(RF_Transactional);
		BakedMergedMesh = MeshToApply;
		Modify();
		MarkPackageDirty();
	}
#endif

	CachedOwnerMesh->SetSkeletalMeshAsset(MeshToApply);
	return true;
}

bool USacraEnemyLoadoutComponent::EffectContainsFire(const FEffect& Effect) const
{
	return Effect.EffectTypes.Contains(EEffectType::Fire);
}

#if WITH_EDITOR
bool USacraEnemyLoadoutComponent::RebuildEditorLoadout()
{
	ResetLoadoutState(true, false);

	if (!ResolveOwnerMesh())
	{
		return false;
	}

	CaptureSourceMeshIfNeeded();
	RestoreSourceMeshIfPossible();

	if (!LoadoutDataAsset)
	{
		Modify();
		MarkPackageDirty();
		return true;
	}

	const FSacraEnemyAppearancePreset* AppearancePreset = SelectAppearanceOption(false);
	if (!AppearancePreset)
	{
		return false;
	}

	if (!ApplyMergedAppearance(*AppearancePreset, true))
	{
		return false;
	}

	if (LoadoutDataAsset->MeshPhysicsAsset)
	{
		CachedOwnerMesh->SetPhysicsAsset(LoadoutDataAsset->MeshPhysicsAsset);
	}

	if (!LoadoutDataAsset->MeshCollisionProfileName.IsNone())
	{
		CachedOwnerMesh->SetCollisionProfileName(LoadoutDataAsset->MeshCollisionProfileName);
	}

	Modify();
	CachedOwnerMesh->Modify();
	MarkPackageDirty();
	return true;
}

bool USacraEnemyLoadoutComponent::RerollEditorLoadout()
{
	ResetLoadoutState(true, true);
	return RebuildEditorLoadout();
}
#endif
