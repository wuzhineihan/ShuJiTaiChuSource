// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PCWindowVaultComponent.generated.h"

class ABasePCPlayer;
class UCameraComponent;
class UWindowVaultVolumeComponent;
class UCapsuleComponent;
class UPrimitiveComponent;

UENUM()
enum class EPCWindowVaultPhase : uint8
{
	None,
	ToStart,
	ToApex,
	ToEnd
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VRTEST_API UPCWindowVaultComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPCWindowVaultComponent();

	UFUNCTION(BlueprintCallable, Category = "Vault")
	bool TryStartVaultBySight(UPrimitiveComponent* SightHitComponent);

	UFUNCTION(BlueprintCallable, Category = "Vault")
	bool IsVaulting() const { return bVaulting; }

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	bool BuildVaultPath(UWindowVaultVolumeComponent* VaultVolume, FVector& OutStart, FVector& OutApex, FVector& OutEnd) const;
	bool TraceGroundAtXY(const FVector2D& XY, float TraceDistance, float& OutGroundZ) const;
	void StartVault(UWindowVaultVolumeComponent* VaultVolume, const FVector& Start, const FVector& Apex, const FVector& End);
	void FinishVault();
	void UpdateVault(float DeltaTime);
	void AdvancePhase();
	FVector CurrentTargetForPhase() const;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vault", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
	float VaultCooldown = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vault", meta=(AllowPrivateAccess="true", ClampMin="10.0"))
	float MaxVaultSightDistance = 350.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vault|Debug", meta=(AllowPrivateAccess="true"))
	bool bDrawDebug = false;

	UPROPERTY(Transient)
	TObjectPtr<ABasePCPlayer> OwnerPlayer = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UCameraComponent> OwnerCamera = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UCapsuleComponent> OwnerCapsule = nullptr;

	UPROPERTY(Transient)
	bool bVaulting = false;

	UPROPERTY(Transient)
	EPCWindowVaultPhase VaultPhase = EPCWindowVaultPhase::None;

	UPROPERTY(Transient)
	float PhaseElapsed = 0.0f;

	UPROPERTY(Transient)
	float LastVaultEndTime = -1000.0f;

	UPROPERTY(Transient)
	FVector PhaseStartCameraLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	FVector VaultStart = FVector::ZeroVector;

	UPROPERTY(Transient)
	FVector VaultApex = FVector::ZeroVector;

	UPROPERTY(Transient)
	FVector VaultEnd = FVector::ZeroVector;

	UPROPERTY(Transient)
	float CameraToCapsuleBottomOffset = 0.0f;

	UPROPERTY(Transient)
	FName CachedCapsuleCollisionProfile = NAME_None;

	UPROPERTY(Transient)
	float DurationToStart = 0.12f;

	UPROPERTY(Transient)
	float DurationToApex = 0.2f;

	UPROPERTY(Transient)
	float DurationToEnd = 0.2f;
};
