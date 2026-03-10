// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerClimbComponent.generated.h"

class AActor;
class ABasePlayer;
class UPlayerGrabHand;

USTRUCT()
struct FPlayerClimbHandRecord
{
	GENERATED_BODY()

	UPROPERTY()
	FVector GrabPointWorld = FVector::ZeroVector;

	UPROPERTY()
	bool bIsRightHand = false;

	UPROPERTY()
	TWeakObjectPtr<AActor> ClimbActor;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VRTEST_API UPlayerClimbComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerClimbComponent();

	UFUNCTION(BlueprintCallable, Category = "Climb")
	void RegisterClimbGrip(UPlayerGrabHand* Hand, AActor* ClimbActor);

	UFUNCTION(BlueprintCallable, Category = "Climb")
	void UnregisterClimbGrip(UPlayerGrabHand* Hand, AActor* ClimbActor);

	UFUNCTION(BlueprintCallable, Category = "Climb")
	bool HasAnyValidClimbGrip();

	UFUNCTION(BlueprintCallable, Category = "Climb")
	void TryExitClimbStateIfNoValidGrip();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void BeginLandingRecover();
	void UpdateLandingRecover(float DeltaSeconds);
	void StopClimb();

	void HandleVRClimb();
	void EnforcePCReachConstraints();

	bool ApplyPlayerOffset(const FVector& Delta, bool bSweep) const;
	bool TraceGroundZ(float& OutGroundZ) const;

	void CleanupInvalidHands();
	void PromoteActiveHand(UPlayerGrabHand* PreferredHand = nullptr);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb|VR", meta=(ClampMin="1.0"))
	float VRMaxPullPerTick = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb|PC", meta=(ClampMin="1.0"))
	float PCArmReachRadius = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb|PC", meta=(ClampMin="1.0"))
	float PCMaxCorrectionPerTick = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb|PC", meta=(ClampMin="1", ClampMax="8"))
	int32 PCConstraintIterations = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb|Landing", meta=(ClampMin="10.0"))
	float LandingTraceDistance = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb|Landing", meta=(ClampMin="0.01"))
	float LandingRecoverDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb|Landing", meta=(ClampMin="0.0"))
	float LandingClearance = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb|Landing", meta=(ClampMin="1.0"))
	float LandingMaxRaisePerTick = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb|Landing", meta=(ClampMin="0.01"))
	float LandingNoProgressTimeout = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb|Landing", meta=(ClampMin="0.0"))
	float LandingMinProgressPerTick = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb|Debug")
	bool bDebugDraw = true;

	UPROPERTY(Transient)
	TObjectPtr<ABasePlayer> OwnerPlayer = nullptr;

	TMap<TWeakObjectPtr<UPlayerGrabHand>, FPlayerClimbHandRecord> HandRecords;
	TWeakObjectPtr<UPlayerGrabHand> ActiveHand;

	bool bLandingRecover = false;
	float LandingElapsed = 0.0f;
	float LandingStartZ = 0.0f;
	float LandingTargetZ = 0.0f;
	float LandingNoProgressElapsed = 0.0f;
};
