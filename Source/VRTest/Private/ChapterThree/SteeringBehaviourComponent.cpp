#include "ChapterThree/SteeringBehaviourComponent.h"

#include "GameFramework/Pawn.h"

USteeringBehaviourComponent::USteeringBehaviourComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USteeringBehaviourComponent::BeginPlay()
{
	Super::BeginPlay();
	Initial();
}

void USteeringBehaviourComponent::AddChaseVector(const FVector& TargetLocation)
{
	if (!MyPawn)
	{
		return;
	}

	ChaseVector = (TargetLocation - MyPawn->GetActorLocation()).GetSafeNormal() * MoveForceParameter;
}

void USteeringBehaviourComponent::AddAwayVector(const FVector& InAwayVector)
{
	AwayVector = InAwayVector * AvoidForceParameter;
}

FSteeringMoveResult USteeringBehaviourComponent::ReturnMoveResult()
{
	return CalculateInput();
}

void USteeringBehaviourComponent::Initial()
{
	MyPawn = Cast<APawn>(GetOwner());
}

FSteeringMoveResult USteeringBehaviourComponent::CalculateInput()
{
	CalculateMoveVector();

	FSteeringMoveResult Result;
	if (!MyPawn || MoveVector.IsNearlyZero())
	{
		return Result;
	}

	const FVector MoveDirection = MoveVector.GetSafeNormal();
	Result.ForwardInput = FVector::DotProduct(MyPawn->GetActorForwardVector(), MoveDirection) >= 0.0f ? 1.0f : -1.0f;

	const float MoveSize = MoveVector.Size();
	if (MoveSize <= KINDA_SMALL_NUMBER)
	{
		return Result;
	}

	const float RightDot = FVector::DotProduct(MyPawn->GetActorRightVector(), MoveVector);
	const float RightRatio = RightDot / MoveSize;
	if (FMath::Abs(RightRatio) > RightInputDeadZone)
	{
		Result.RightInput = FMath::Sign(RightRatio);
	}

	return Result;
}

void USteeringBehaviourComponent::CalculateMoveVector()
{
	MoveVector = AwayVector + ChaseVector;
}
