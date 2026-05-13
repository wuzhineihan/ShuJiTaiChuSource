// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/PCClimbLadderComponent.h"

#include "Components/CapsuleComponent.h"
#include "Game/Characters/BasePCPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Scene/LadderVolumeComponent.h"

UPCClimbLadderComponent::UPCClimbLadderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPCClimbLadderComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerPlayer = Cast<ABasePCPlayer>(GetOwner());
	if (!OwnerPlayer)
	{
		return;
	}

	OwnerMovement = OwnerPlayer->GetCharacterMovement();

	TArray<UActorComponent*> Components;
	OwnerPlayer->GetComponents(UCapsuleComponent::StaticClass(), Components);
	for (UActorComponent* Component : Components)
	{
		if (UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(Component))
		{
			Capsule->OnComponentBeginOverlap.AddDynamic(this, &UPCClimbLadderComponent::OnLadderBeginOverlap);
			Capsule->OnComponentEndOverlap.AddDynamic(this, &UPCClimbLadderComponent::OnLadderEndOverlap);
		}
	}
}

void UPCClimbLadderComponent::HandleMoveInput(const FVector& ForwardDir, const FVector& RightDir, const FVector2D& MoveInput)
{
	if (!OwnerPlayer || !OwnerMovement)
	{
		return;
	}

	const FVector SafeForward = ForwardDir.GetSafeNormal2D();
	const FVector SafeRight = RightDir.GetSafeNormal2D();

	const FVector DesiredMoveWorld2D = (SafeForward * MoveInput.Y) + (SafeRight * MoveInput.X);
	const float InputMagnitude = FMath::Clamp(FMath::Sqrt((MoveInput.X * MoveInput.X) + (MoveInput.Y * MoveInput.Y)), 0.0f, 1.0f);
	const float NowTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	if (!bIsOnLadder && (NowTime - LastLadderExitTime) < LadderReattachCooldown)
	{
		OwnerPlayer->AddMovementInput(SafeRight, MoveInput.X);
		OwnerPlayer->AddMovementInput(SafeForward, MoveInput.Y);
		return;
	}

	if (!CurrentLadder || !IsTouchingCurrentLadder())
	{
		ExitLadderMode();
		OwnerPlayer->AddMovementInput(SafeRight, MoveInput.X);
		OwnerPlayer->AddMovementInput(SafeForward, MoveInput.Y);
		return;
	}

	const FVector LadderNormal = CurrentLadder->GetLadderNormal().GetSafeNormal2D();
	const FVector LadderUp = CurrentLadder->GetLadderUp();
	if (LadderNormal.IsNearlyZero() || LadderUp.IsNearlyZero())
	{
		ExitLadderMode();
		OwnerPlayer->AddMovementInput(SafeRight, MoveInput.X);
		OwnerPlayer->AddMovementInput(SafeForward, MoveInput.Y);
		return;
	}

	const FVector MoveDirWorld = DesiredMoveWorld2D.GetSafeNormal2D();
	const bool bTowardLadder = ShouldClimbTowardLadder(MoveDirWorld, LadderNormal);
	const bool bHasInput = InputMagnitude > KINDA_SMALL_NUMBER;

	if (!bHasInput)
	{
		ExitLadderMode();
		return;
	}

	if (!bTowardLadder)
	{
		ExitLadderMode();
		OwnerPlayer->AddMovementInput(SafeRight, MoveInput.X);
		OwnerPlayer->AddMovementInput(SafeForward, MoveInput.Y);
		ExitLadderIfNeeded();
		return;
	}

	if (IsNearLadderTop() && MoveInput.Y > 0.0f)
	{
		ExitLadderMode();
		OwnerPlayer->AddMovementInput(SafeForward, MoveInput.Y);
		OwnerPlayer->AddMovementInput(SafeRight, MoveInput.X);
		return;
	}

	EnterLadderMode();

	const float TowardAmount = FMath::Clamp(FVector::DotProduct(MoveDirWorld, -LadderNormal), 0.0f, 1.0f);
	const float ClimbScale = TowardAmount * InputMagnitude * LadderClimbSpeedScale;
	OwnerPlayer->AddMovementInput(LadderUp, ClimbScale);

	const FVector LadderRight = FVector::CrossProduct(LadderUp, LadderNormal).GetSafeNormal();
	const float StrafeAmount = FVector::DotProduct(DesiredMoveWorld2D, LadderRight);
	if (FMath::Abs(StrafeAmount) > KINDA_SMALL_NUMBER)
	{
		OwnerPlayer->AddMovementInput(LadderRight, FMath::Clamp(StrafeAmount, -1.0f, 1.0f));
	}
}

void UPCClimbLadderComponent::OnLadderBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	ULadderVolumeComponent* LadderVolume = Cast<ULadderVolumeComponent>(OtherComp);
	if (!LadderVolume || OtherActor == nullptr)
	{
		return;
	}

	CurrentLadder = LadderVolume;
}

void UPCClimbLadderComponent::OnLadderEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (!CurrentLadder || OtherComp != CurrentLadder)
	{
		return;
	}

	CurrentLadder = nullptr;
	ExitLadderMode();
}

bool UPCClimbLadderComponent::IsTouchingCurrentLadder() const
{
	if (!OwnerPlayer || !CurrentLadder)
	{
		return false;
	}

	return CurrentLadder->IsOverlappingActor(OwnerPlayer);
}

bool UPCClimbLadderComponent::ShouldClimbTowardLadder(const FVector& MoveDirWorld, const FVector& LadderNormal) const
{
	if (MoveDirWorld.IsNearlyZero() || LadderNormal.IsNearlyZero())
	{
		return false;
	}

	const float Toward = FVector::DotProduct(MoveDirWorld, -LadderNormal);
	return Toward >= ClimbTowardThreshold;
}

void UPCClimbLadderComponent::ExitLadderIfNeeded()
{
	if (!CurrentLadder || !OwnerPlayer)
	{
		return;
	}

	if (!CurrentLadder->IsOverlappingActor(OwnerPlayer))
	{
		CurrentLadder = nullptr;
		ExitLadderMode();
	}
}

void UPCClimbLadderComponent::EnterLadderMode()
{
	if (!OwnerMovement)
	{
		return;
	}

	if (!bIsOnLadder)
	{
		CachedMovementMode = OwnerMovement->MovementMode;
		bHasCachedMovementMode = true;
		OwnerMovement->SetMovementMode(MOVE_Flying);
	}

	bIsOnLadder = true;
}

void UPCClimbLadderComponent::ExitLadderMode()
{
	if (!OwnerMovement || !bIsOnLadder)
	{
		bIsOnLadder = false;
		return;
	}

	const EMovementMode TargetMode = bHasCachedMovementMode ? CachedMovementMode.GetValue() : MOVE_Walking;
	OwnerMovement->SetMovementMode(TargetMode);
	bHasCachedMovementMode = false;
	bIsOnLadder = false;
	LastLadderExitTime = GetWorld() ? GetWorld()->GetTimeSeconds() : LastLadderExitTime;
}

bool UPCClimbLadderComponent::IsNearLadderTop() const
{
	if (!OwnerPlayer || !CurrentLadder)
	{
		return false;
	}

	const FTransform BoxTransform = CurrentLadder->GetComponentTransform();
	const FVector PlayerLocation = OwnerPlayer->GetActorLocation();
	const FVector LocalPlayer = BoxTransform.InverseTransformPosition(PlayerLocation);
	const FVector Extent = CurrentLadder->GetScaledBoxExtent();
	const float DistanceToTop = Extent.Z - LocalPlayer.Z + OwnerPlayer->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	return DistanceToTop <= LadderTopExitThreshold;
}
