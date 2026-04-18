// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EditorStaticMeshUtils.generated.h"

class UStaticMesh;

UCLASS()
class VRTEST_API UEditorStaticMeshUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Editor Utility: rotate all matching StaticMeshActors in current map by random yaw delta.
	 * Only works in editor world.
	 *
	 * @param WorldContextObject Editor Utility Widget / Blutility self is acceptable.
	 * @param TargetMeshes StaticMesh assets to match in map.
	 * @param MinYawDelta Random yaw delta minimum (degrees).
	 * @param MaxYawDelta Random yaw delta maximum (degrees).
	 * @param RandomSeed Random seed (same seed => deterministic result).
	 * @return Number of actors rotated.
	 */
	UFUNCTION(BlueprintCallable, Category = "Editor|StaticMesh", meta = (WorldContext = "WorldContextObject", DevelopmentOnly))
	static int32 ApplyRandomYawToStaticMeshes(
		UObject* WorldContextObject,
		const TArray<UStaticMesh*>& TargetMeshes,
		float MinYawDelta = -180.0f,
		float MaxYawDelta = 180.0f,
		int32 RandomSeed = 12345
	);
};
