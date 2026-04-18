// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/EditorStaticMeshUtils.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"

int32 UEditorStaticMeshUtils::ApplyRandomYawToStaticMeshes(
	UObject* WorldContextObject,
	const TArray<UStaticMesh*>& TargetMeshes,
	float MinYawDelta,
	float MaxYawDelta,
	int32 RandomSeed)
{
#if !WITH_EDITOR
	return 0;
#else
	if (!WorldContextObject || TargetMeshes.Num() <= 0)
	{
		return 0;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return 0;
	}

	if (MinYawDelta > MaxYawDelta)
	{
		Swap(MinYawDelta, MaxYawDelta);
	}

	TSet<TObjectPtr<const UStaticMesh>> TargetSet;
	for (UStaticMesh* Mesh : TargetMeshes)
	{
		if (Mesh)
		{
			TargetSet.Add(Mesh);
		}
	}
	if (TargetSet.Num() <= 0)
	{
		return 0;
	}

	FRandomStream RNG(RandomSeed);
	int32 RotatedCount = 0;

	for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
	{
		AStaticMeshActor* MeshActor = *It;
		if (!MeshActor)
		{
			continue;
		}

		UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent();
		if (!MeshComp)
		{
			continue;
		}

		const UStaticMesh* MeshAsset = MeshComp->GetStaticMesh();
		if (!MeshAsset || !TargetSet.Contains(MeshAsset))
		{
			continue;
		}

		const float DeltaYaw = RNG.FRandRange(MinYawDelta, MaxYawDelta);
		FRotator NewRotation = MeshActor->GetActorRotation();
		NewRotation.Yaw += DeltaYaw;

		MeshActor->Modify();
		MeshActor->SetActorRotation(NewRotation);
		MeshActor->MarkPackageDirty();
		++RotatedCount;
	}

	return RotatedCount;
#endif
}
