// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Perception/AISense.h"

#include "AISense_Player.generated.h"

class ABasePlayer;
class UAISenseConfig_Player;

UCLASS(ClassGroup = AI, config = Game)
class VRTEST_API UAISense_Player : public UAISense
{
	GENERATED_UCLASS_BODY()

	struct FDigestedPlayerProperties
	{
		float PlayerRadius = 1000.0f;
		float PlayerSightDegree = PI / 3.0f;
		float GrassSightRadius = 150.0f;
		bool bEnableDebugDraw = false;
		float DebugDrawDuration = 0.0f;
		float DebugLineThickness = 1.0f;
		FColor DebugVisibleColor = FColor::Green;
		FColor DebugBlockedColor = FColor::Red;
		FColor DebugRangeColor = FColor::Green;

		bool bHasVisibleTarget = false;
		FVector LastTargetLocation = FVector::ZeroVector;
		TWeakObjectPtr<AActor> LastTargetActor = nullptr;

		FDigestedPlayerProperties() = default;
		explicit FDigestedPlayerProperties(const UAISenseConfig_Player& SenseConfig);
	};

protected:
	virtual float Update() override;

	void OnNewListenerImpl(const FPerceptionListener& NewListener);
	void OnListenerUpdateImpl(const FPerceptionListener& UpdatedListener);
	void OnListenerRemovedImpl(const FPerceptionListener& RemovedListener);

private:
	ABasePlayer* ResolvePlayerCharacter(const UWorld* World) const;
	bool CheckTargetInRange(const ABasePlayer* InTarget, float& OutStrength, const FPerceptionListener& Listener) const;
	bool PerformLineOfSightCheck(const ABasePlayer* TargetPlayer, const FPerceptionListener& Listener, FVector& OutSeenLocation, float& OutSightStrength) const;
	void DrawDebugInfo(const FPerceptionListener& Listener, const ABasePlayer* TargetPlayer, bool bInRange, bool bHasLineOfSight, const FVector& SeenLocation) const;

private:
	TMap<FPerceptionListenerID, FDigestedPlayerProperties> DigestedProperties;
};
