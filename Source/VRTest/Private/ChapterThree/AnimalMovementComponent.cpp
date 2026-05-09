#include "ChapterThree/AnimalMovementComponent.h"

#include "Curves/CurveFloat.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"

UAnimalMovementComponent::UAnimalMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAnimalMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	Initial();
}

void UAnimalMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateMovementInput(DeltaTime);
}

void UAnimalMovementComponent::AddHorseInput(float InRightInput, float InForwardInput, float InSpeedAcceleration, float InYawRotationAcceleration)
{
	RightInput = InRightInput;
	ForwardInput = InForwardInput;
	SetHorseSpeedAcceleration(InSpeedAcceleration);
	SetHorseYawRotationAcceleration(InYawRotationAcceleration);
}

void UAnimalMovementComponent::SetHorseSpeedAcceleration(float InSpeedAcceleration)
{
	SpeedAcceleration = FMath::Max(0.0f, InSpeedAcceleration);
}

void UAnimalMovementComponent::SetHorseYawRotationAcceleration(float InYawRotationAcceleration)
{
	YawRotationAcceleration = FMath::Max(0.0f, InYawRotationAcceleration);
}

void UAnimalMovementComponent::SetMaxSpeed(float InMaxSpeed)
{
	MaxSpeed = FMath::Max(0.0f, InMaxSpeed);
	TargetMaxSpeed = FMath::Clamp(TargetMaxSpeed, 0.0f, MaxSpeed);
	CurrentMaxSpeed = FMath::Clamp(CurrentMaxSpeed, 0.0f, MaxSpeed);
}

void UAnimalMovementComponent::Stop()
{
	RightInput = 0.0f;
	ForwardInput = 0.0f;
	TargetMaxSpeed = 0.0f;
	CurrentMaxSpeed = 0.0f;
	TargetYawRotationRate = 0.0f;
	CurrentYawRotationRate = 0.0f;

	if (MyCharacterMovement)
	{
		MyCharacterMovement->MaxWalkSpeed = 0.0f;
		MyCharacterMovement->StopMovementImmediately();
	}
}

void UAnimalMovementComponent::Initial()
{
	MyPawn = Cast<APawn>(GetOwner());
	if (MyPawn)
	{
		MyCharacterMovement = MyPawn->FindComponentByClass<UCharacterMovementComponent>();
	}
}

void UAnimalMovementComponent::UpdateMovementInput(float DeltaTime)
{
	Debug();

	if (!MyPawn || !MyCharacterMovement)
	{
		return;
	}

	const bool bIdleState = FMath::IsNearlyZero(RightInput)
		&& FMath::IsNearlyZero(ForwardInput)
		&& FMath::IsNearlyZero(TargetMaxSpeed)
		&& FMath::IsNearlyZero(CurrentMaxSpeed)
		&& FMath::IsNearlyZero(TargetYawRotationRate)
		&& FMath::IsNearlyZero(CurrentYawRotationRate);
	if (bIdleState)
	{
		return;
	}

	const bool bPivotOnly = FMath::IsNearlyZero(ForwardInput) && !FMath::IsNearlyZero(RightInput) && FMath::IsNearlyZero(CurrentMaxSpeed);
	if (bPivotOnly)
	{
		TargetMaxSpeed = 0.0f;
		CurrentMaxSpeed = FMath::FInterpTo(CurrentMaxSpeed, 0.0f, DeltaTime, PivotInterpRate);
	}
	else
	{
		TargetMaxSpeed = FMath::Clamp(TargetMaxSpeed + ForwardInput * DeltaTime * SpeedAcceleration, 0.0f, MaxSpeed);

		const bool bShouldBrakeToIdle = FMath::IsNearlyZero(ForwardInput) && TargetMaxSpeed <= WalkSpeed;
		const float SpeedTarget = bShouldBrakeToIdle ? 0.0f : TargetMaxSpeed;
		const float InterpRate = bShouldBrakeToIdle ? SlowdownInterpRate : SpeedInterpRate;
		CurrentMaxSpeed = FMath::FInterpTo(CurrentMaxSpeed, SpeedTarget, DeltaTime, InterpRate);
	}

	MyCharacterMovement->MaxWalkSpeed = FMath::Abs(CurrentMaxSpeed);
	if (!FMath::IsNearlyZero(CurrentMaxSpeed))
	{
		MyPawn->AddMovementInput(MyPawn->GetActorForwardVector(), FMath::Sign(CurrentMaxSpeed));
	}

	const float MaxYawRotationRate = GetMaxYawRotationRate();
	if (FMath::IsNearlyZero(RightInput))
	{
		TargetYawRotationRate = FMath::FInterpTo(TargetYawRotationRate, 0.0f, DeltaTime, YawInterpRate);
	}
	else
	{
		TargetYawRotationRate = FMath::Clamp(
			TargetYawRotationRate + RightInput * DeltaTime * YawRotationAcceleration,
			-MaxYawRotationRate,
			MaxYawRotationRate);
	}

	CurrentYawRotationRate = FMath::FInterpTo(CurrentYawRotationRate, TargetYawRotationRate, DeltaTime, YawInterpRate);
	if (!FMath::IsNearlyZero(CurrentYawRotationRate))
	{
		float MovementSign = 1.0f;
		const FVector VelocityDirection = MyCharacterMovement->Velocity.GetSafeNormal();
		if (!VelocityDirection.IsNearlyZero())
		{
			MovementSign = FMath::Sign(FVector::DotProduct(VelocityDirection, MyPawn->GetActorForwardVector()));
			if (FMath::IsNearlyZero(MovementSign))
			{
				MovementSign = 1.0f;
			}
		}

		MyPawn->AddActorLocalRotation(FRotator(0.0f, CurrentYawRotationRate * DeltaTime * MovementSign, 0.0f));
	}

	// Match the BP behavior by re-applying the latest cached inputs every frame.
	AddHorseInput(RightInput, ForwardInput, SpeedAcceleration, YawRotationAcceleration);
}

void UAnimalMovementComponent::Debug() const
{
	if (!bShowDebug)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("HorseMove CurrentSpeed=%.2f ForwardInput=%.2f CurrentYawRate=%.2f RightInput=%.2f"),
		CurrentMaxSpeed,
		ForwardInput,
		CurrentYawRotationRate,
		RightInput);
}

float UAnimalMovementComponent::GetMaxYawRotationRate() const
{
	if (YawRotationRateCurve)
	{
		return FMath::Max(0.0f, YawRotationRateCurve->GetFloatValue(FMath::Abs(CurrentMaxSpeed)));
	}

	return FallbackMaxYawRotationRate;
}
