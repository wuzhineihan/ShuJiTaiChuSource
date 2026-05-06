// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "SacraEnemyLoadoutDataAsset.generated.h"

class UPhysicsAsset;
class USkeletalMesh;
class USkeleton;

UENUM(BlueprintType)
enum class ESacraEnemyArmorType : uint8
{
	None,
	Heavy
};

USTRUCT(BlueprintType)
struct FSacraEnemyAppearancePreset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	FName AppearanceId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	TArray<TObjectPtr<USkeletalMesh>> MeshesToMerge;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	TObjectPtr<USkeleton> Skeleton = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout", meta = (ClampMin = "0"))
	int32 StripTopLODs = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	bool bNeedsCpuAccess = false;
};

UCLASS(BlueprintType)
class VRTEST_API USacraEnemyLoadoutDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	ESacraEnemyArmorType ArmorType = ESacraEnemyArmorType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	FName MeshCollisionProfileName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	TObjectPtr<UPhysicsAsset> MeshPhysicsAsset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	TArray<FSacraEnemyAppearancePreset> AppearancePresets;
};
