// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/PlayerClimbComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Game/Characters/BasePCPlayer.h"
#include "Game/Characters/BasePlayer.h"
#include "Grabber/PlayerGrabHand.h"
#include "Engine/World.h"

UPlayerClimbComponent::UPlayerClimbComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetComponentTickEnabled(false);
}

void UPlayerClimbComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerPlayer = Cast<ABasePlayer>(GetOwner());
}

void UPlayerClimbComponent::RegisterClimbGrip(UPlayerGrabHand* Hand, AActor* ClimbActor)
{
	if (!OwnerPlayer || !Hand || !ClimbActor || Hand->GetOwner() != OwnerPlayer)
	{
		return;
	}

	CleanupInvalidHands();
	const int32 CountBefore = HandRecords.Num();

	FPlayerClimbHandRecord& Record = HandRecords.FindOrAdd(Hand);
	Record.GrabPointWorld = Hand->GetComponentLocation();
	Record.bIsRightHand = Hand->bIsRightHand;
	Record.ClimbActor = ClimbActor;

	if (const ABasePCPlayer* PCPlayer = Cast<ABasePCPlayer>(OwnerPlayer.Get()))
	{
		if (PCPlayer->TargetedObject == ClimbActor && PCPlayer->bTraceHit)
		{
			Record.GrabPointWorld = PCPlayer->TargetedImpactPoint;
		}
	}

	ActiveHand = Hand;
	bLandingRecover = false;
	LandingElapsed = 0.0f;

	if (CountBefore <= 0 && HandRecords.Num() > 0)
	{
		OwnerPlayer->EnterClimbState();
	}

	SetComponentTickEnabled(true);
}

void UPlayerClimbComponent::UnregisterClimbGrip(UPlayerGrabHand* Hand, AActor* ClimbActor)
{
	if (!OwnerPlayer || !Hand)
	{
		return;
	}

	CleanupInvalidHands();

	if (FPlayerClimbHandRecord* Existing = HandRecords.Find(Hand))
	{
		if (!ClimbActor || !Existing->ClimbActor.IsValid() || Existing->ClimbActor.Get() == ClimbActor)
		{
			HandRecords.Remove(Hand);
		}
	}
	else
	{
		HandRecords.Remove(Hand);
	}

	CleanupInvalidHands();

	if (HandRecords.Num() > 0)
	{
		PromoteActiveHand(Hand->OtherHand);
		SetComponentTickEnabled(true);
		return;
	}

	BeginLandingRecover();
}

bool UPlayerClimbComponent::HasAnyValidClimbGrip()
{
	CleanupInvalidHands();
	return HandRecords.Num() > 0;
}

void UPlayerClimbComponent::TryExitClimbStateIfNoValidGrip()
{
	CleanupInvalidHands();

	if (!OwnerPlayer)
	{
		return;
	}

	if (HandRecords.Num() <= 0 && !bLandingRecover)
	{
		OwnerPlayer->ExitClimbState();
	}
}

void UPlayerClimbComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OwnerPlayer)
	{
		SetComponentTickEnabled(false);
		return;
	}

	CleanupInvalidHands();

	if (bLandingRecover)
	{
		if (HandRecords.Num() > 0)
		{
			bLandingRecover = false;
			LandingElapsed = 0.0f;
			return;
		}

		UpdateLandingRecover(DeltaTime);
		return;
	}

	if (HandRecords.Num() <= 0)
	{
		BeginLandingRecover();
		return;
	}

	if (!ActiveHand.IsValid() || !HandRecords.Contains(ActiveHand))
	{
		PromoteActiveHand();
	}

	if (Cast<ABasePCPlayer>(OwnerPlayer.Get()))
	{
		EnforcePCReachConstraints();
	}
	else
	{
		HandleVRClimb();
	}
}

void UPlayerClimbComponent::BeginLandingRecover()
{
	if (!OwnerPlayer)
	{
		StopClimb();
		return;
	}

	if (HandRecords.Num() > 0)
	{
		return;
	}

	const UCapsuleComponent* Capsule = OwnerPlayer->GetCapsuleComponent();
	if (!Capsule)
	{
		StopClimb();
		return;
	}

	float GroundZ = 0.0f;
	if (!TraceGroundZ(GroundZ))
	{
		StopClimb();
		return;
	}

	const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	// Use camera height as the landing reference to match player perception.
	const float CurrentBottomZ = Capsule->GetComponentLocation().Z - HalfHeight;
	const float DesiredBottomZ = GroundZ + LandingClearance;
	if (DesiredBottomZ <= CurrentBottomZ + KINDA_SMALL_NUMBER)
	{
		StopClimb();
		return;
	}

	bLandingRecover = true;
	LandingElapsed = 0.0f;
	LandingStartZ = OwnerPlayer->GetActorLocation().Z;
	LandingTargetZ = LandingStartZ + (DesiredBottomZ - CurrentBottomZ);
	SetComponentTickEnabled(true);
}

void UPlayerClimbComponent::UpdateLandingRecover(float DeltaSeconds)
{
	if (!OwnerPlayer)
	{
		StopClimb();
		return;
	}

	LandingElapsed += DeltaSeconds;
	const float Alpha = LandingRecoverDuration > 0.0f
		? FMath::Clamp(LandingElapsed / LandingRecoverDuration, 0.0f, 1.0f)
		: 1.0f;

	const float DesiredZ = (Alpha < 1.0f)
		? FMath::Lerp(LandingStartZ, LandingTargetZ, Alpha)
		: LandingTargetZ;
	const float CurrentZ = OwnerPlayer->GetActorLocation().Z;
	const float DeltaZRaw = DesiredZ - CurrentZ;
	const float DeltaZ = FMath::Clamp(DeltaZRaw, -LandingMaxRaisePerTick, LandingMaxRaisePerTick);

	if (DeltaZ > KINDA_SMALL_NUMBER)
	{
		ApplyPlayerOffset(FVector(0.0f, 0.0f, DeltaZ), true);
	}

	if (FMath::Abs(LandingTargetZ - OwnerPlayer->GetActorLocation().Z) <= 0.5f)
	{
		StopClimb();
	}
}

void UPlayerClimbComponent::StopClimb()
{
	const bool bHadAnyGrip = HandRecords.Num() > 0;

	bLandingRecover = false;
	LandingElapsed = 0.0f;
	LandingStartZ = 0.0f;
	LandingTargetZ = 0.0f;
	ActiveHand.Reset();
	HandRecords.Reset();

	if (OwnerPlayer && !bHadAnyGrip)
	{
		OwnerPlayer->ExitClimbState();
	}

	SetComponentTickEnabled(false);
}

void UPlayerClimbComponent::HandleVRClimb()
{
	if (!OwnerPlayer || !ActiveHand.IsValid())
	{
		return;
	}

	FPlayerClimbHandRecord* ActiveRecord = HandRecords.Find(ActiveHand);
	if (!ActiveRecord)
	{
		return;
	}

	const FVector HandLocation = ActiveHand->GetComponentLocation();
	FVector PullDelta = ActiveRecord->GrabPointWorld - HandLocation;
	PullDelta = PullDelta.GetClampedToMaxSize(VRMaxPullPerTick);

	if (!PullDelta.IsNearlyZero())
	{
		ApplyPlayerOffset(PullDelta, true);
	}

	if (bDebugDraw)
	{
		DrawDebugSphere(GetWorld(), ActiveRecord->GrabPointWorld, 6.0f, 8, FColor::Cyan, false, 0.0f);
		DrawDebugLine(GetWorld(), ActiveRecord->GrabPointWorld, HandLocation, FColor::Cyan, false, 0.0f, 0, 1.5f);
	}
}

void UPlayerClimbComponent::EnforcePCReachConstraints()
{
	if (!OwnerPlayer)
	{
		return;
	}

	const ABasePCPlayer* PCPlayer = Cast<ABasePCPlayer>(OwnerPlayer.Get());
	const UCameraComponent* CameraComp = PCPlayer ? PCPlayer->FirstPersonCamera : nullptr;
	const FVector CameraLoc = CameraComp ? CameraComp->GetComponentLocation() : OwnerPlayer->GetActorLocation();

	const int32 Iterations = FMath::Max(1, PCConstraintIterations);
	for (int32 Iter = 0; Iter < Iterations; ++Iter)
	{
		FVector TotalCorrection = FVector::ZeroVector;

		if (ActiveHand.IsValid())
		{
			if (const FPlayerClimbHandRecord* ActiveRecord = HandRecords.Find(ActiveHand))
			{
				const FVector ToCamera = CameraLoc - ActiveRecord->GrabPointWorld;
				const float Dist = ToCamera.Size();
				if (Dist > PCArmReachRadius && Dist > KINDA_SMALL_NUMBER)
				{
					const float Excess = Dist - PCArmReachRadius;
					TotalCorrection += -ToCamera.GetSafeNormal() * Excess;
				}
			}
		}

		for (const TPair<TWeakObjectPtr<UPlayerGrabHand>, FPlayerClimbHandRecord>& Pair : HandRecords)
		{
			const TWeakObjectPtr<UPlayerGrabHand>& HandKey = Pair.Key;
			const FPlayerClimbHandRecord& Record = Pair.Value;
			if (!HandKey.IsValid())
			{
				continue;
			}
			if (ActiveHand.IsValid() && HandKey == ActiveHand)
			{
				continue;
			}

			const FVector ToCamera = CameraLoc - Record.GrabPointWorld;
			const float Dist = ToCamera.Size();
			if (Dist > PCArmReachRadius && Dist > KINDA_SMALL_NUMBER)
			{
				const float Excess = Dist - PCArmReachRadius;
				TotalCorrection += -ToCamera.GetSafeNormal() * Excess;
			}
		}

		TotalCorrection = TotalCorrection.GetClampedToMaxSize(PCMaxCorrectionPerTick);
		if (TotalCorrection.IsNearlyZero())
		{
			break;
		}

		ApplyPlayerOffset(TotalCorrection, true);

		if (bDebugDraw)
		{
			DrawDebugDirectionalArrow(
				GetWorld(),
				OwnerPlayer->GetActorLocation(),
				OwnerPlayer->GetActorLocation() + TotalCorrection * 2.0f,
				20.0f,
				FColor::Orange,
				false,
				0.0f,
				0,
				1.0f);
		}
	}

	if (bDebugDraw)
	{
		for (const TPair<TWeakObjectPtr<UPlayerGrabHand>, FPlayerClimbHandRecord>& Pair : HandRecords)
		{
			if (!Pair.Key.IsValid())
			{
				continue;
			}
			DrawDebugSphere(GetWorld(), Pair.Value.GrabPointWorld, PCArmReachRadius, 16, FColor::Green, false, 0.0f);
			DrawDebugLine(GetWorld(), Pair.Value.GrabPointWorld, CameraLoc, FColor::Green, false, 0.0f, 0, 1.0f);
		}
	}
}

bool UPlayerClimbComponent::ApplyPlayerOffset(const FVector& Delta, bool bSweep) const
{
	if (!OwnerPlayer || Delta.IsNearlyZero())
	{
		return false;
	}

	FHitResult Hit;
	OwnerPlayer->AddActorWorldOffset(Delta, bSweep, &Hit, ETeleportType::None);
	return true;
}

bool UPlayerClimbComponent::TraceGroundZ(float& OutGroundZ) const
{
	OutGroundZ = 0.0f;
	if (!OwnerPlayer)
	{
		return false;
	}

	const UCapsuleComponent* Capsule = OwnerPlayer->GetCapsuleComponent();
	if (!Capsule)
	{
		return false;
	}

	const UCameraComponent* CameraComp = OwnerPlayer->PlayerCamera;
	const FVector Start = CameraComp ? CameraComp->GetComponentLocation() : Capsule->GetComponentLocation();
	const FVector End = Start - FVector(0.0f, 0.0f, LandingTraceDistance + Capsule->GetScaledCapsuleHalfHeight() + 10.0f);

	FCollisionObjectQueryParams ObjQuery;
	ObjQuery.AddObjectTypesToQuery(ECC_WorldStatic);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ClimbLandingTrace), false);
	QueryParams.AddIgnoredActor(OwnerPlayer);

	FHitResult Hit;
	const bool bHit = GetWorld()->LineTraceSingleByObjectType(Hit, Start, End, ObjQuery, QueryParams);
	if (!bHit)
	{
		return false;
	}

	OutGroundZ = Hit.ImpactPoint.Z;

	if (bDebugDraw)
	{
		DrawDebugLine(GetWorld(), Start, Hit.ImpactPoint, FColor::Yellow, false, 0.5f, 0, 1.5f);
		DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 8.0f, FColor::Yellow, false, 0.5f);
	}

	return true;
}

void UPlayerClimbComponent::CleanupInvalidHands()
{
	TArray<TWeakObjectPtr<UPlayerGrabHand>> InvalidHands;
	for (const TPair<TWeakObjectPtr<UPlayerGrabHand>, FPlayerClimbHandRecord>& Pair : HandRecords)
	{
		const UPlayerGrabHand* Hand = Pair.Key.Get();
		const AActor* ClimbActor = Pair.Value.ClimbActor.Get();
		if (!Pair.Key.IsValid() || !Pair.Value.ClimbActor.IsValid() || !Hand || !ClimbActor || !Hand->bIsHolding || Hand->HeldActor != ClimbActor)
		{
			InvalidHands.Add(Pair.Key);
		}
	}

	for (const TWeakObjectPtr<UPlayerGrabHand>& Hand : InvalidHands)
	{
		HandRecords.Remove(Hand);
	}
}

void UPlayerClimbComponent::PromoteActiveHand(UPlayerGrabHand* PreferredHand)
{
	if (PreferredHand)
	{
		for (const TPair<TWeakObjectPtr<UPlayerGrabHand>, FPlayerClimbHandRecord>& Pair : HandRecords)
		{
			if (Pair.Key.Get() == PreferredHand)
			{
				ActiveHand = Pair.Key;
				return;
			}
		}
	}

	for (const TPair<TWeakObjectPtr<UPlayerGrabHand>, FPlayerClimbHandRecord>& Pair : HandRecords)
	{
		if (Pair.Key.IsValid())
		{
			ActiveHand = Pair.Key;
			return;
		}
	}

	ActiveHand.Reset();
}
