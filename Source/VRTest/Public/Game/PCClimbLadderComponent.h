// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PCClimbLadderComponent.generated.h"

class ABasePCPlayer;
class ALadderVolume;
class UBoxComponent;
class UCharacterMovementComponent;
class UPrimitiveComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VRTEST_API UPCClimbLadderComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPCClimbLadderComponent();

	UFUNCTION(BlueprintCallable, Category = "Ladder")
	void HandleMoveInput(const FVector& ForwardDir, const FVector& RightDir, const FVector2D& MoveInput);

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnLadderBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnLadderEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	bool IsTouchingCurrentLadder() const;
	bool ShouldClimbTowardLadder(const FVector& MoveDirWorld, const FVector& LadderNormal) const;
	void ExitLadderIfNeeded();
	void EnterLadderMode();
	void ExitLadderMode();
	bool IsNearLadderTop() const;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ladder", meta=(AllowPrivateAccess="true", ClampMin="0.0", ClampMax="1.0"))
	float ClimbTowardThreshold = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ladder", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
	float LadderClimbSpeedScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ladder", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
	float LadderTopExitThreshold = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ladder", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
	float LadderReattachCooldown = 0.2f;

	UPROPERTY(Transient)
	TObjectPtr<ABasePCPlayer> OwnerPlayer = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UCharacterMovementComponent> OwnerMovement = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<ALadderVolume> CurrentLadder = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UBoxComponent> CurrentLadderBox = nullptr;

	UPROPERTY(Transient)
	bool bIsOnLadder = false;

	UPROPERTY(Transient)
	TEnumAsByte<EMovementMode> CachedMovementMode = MOVE_Walking;

	UPROPERTY(Transient)
	bool bHasCachedMovementMode = false;

	UPROPERTY(Transient)
	float LastLadderExitTime = -1000.0f;
};
