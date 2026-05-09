#include "ChapterThree/CartHorseBase.h"

#include "ChapterThree/AnimalMovementComponent.h"
#include "ChapterThree/Carriage.h"
#include "ChapterThree/SteeringBehaviourComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

ACartHorseBase::ACartHorseBase()
{
	PrimaryActorTick.bCanEverTick = true;

	HorseMovementComponent = CreateDefaultSubobject<UAnimalMovementComponent>(TEXT("HorseMovementComponent"));
	SteeringBehaviourComponent = CreateDefaultSubobject<USteeringBehaviourComponent>(TEXT("SteeringBehaviourComponent"));
}

void ACartHorseBase::BeginPlay()
{
	Super::BeginPlay();

	if (HorseMovementComponent)
	{
		HorseMovementComponent->SetMaxSpeed(DefaultMaxSpeed);
	}

	if (AvoidanceCheckInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(
			AvoidanceTimerHandle,
			this,
			&ACartHorseBase::UpdateAvoidenceMode,
			AvoidanceCheckInterval,
			true);
	}
}

void ACartHorseBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorldTimerManager().ClearTimer(AvoidanceTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void ACartHorseBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bCanMove)
	{
		CartForward();
	}
	else if (HorseMovementComponent)
	{
		HorseMovementComponent->AddHorseInput(0.0f, -1.0f, DefaultSpeedAcceleration, DefaultYawRotationAcceleration);
	}
}

void ACartHorseBase::SetMovable(bool bMovable)
{
	bCanMove = bMovable;

	if (!bCanMove && HorseMovementComponent)
	{
		HorseMovementComponent->AddHorseInput(0.0f, -1.0f, DefaultSpeedAcceleration, DefaultYawRotationAcceleration);
	}
}

AActor* ACartHorseBase::GetCart()
{
	if (IsValid(Cart))
	{
		return Cart;
	}

	if (const ACarriage* ParentCarriage = Cast<ACarriage>(GetParentActor()))
	{
		Cart = ParentCarriage->CartActor;
	}

	return Cart;
}

FVector ACartHorseBase::CheckAvoidence(int32 LineTraceTimes)
{
	if (LineTraceTimes <= 0)
	{
		return FVector::ZeroVector;
	}

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	if (AActor* CartActor = GetCart())
	{
		ActorsToIgnore.Add(CartActor);
	}

	FVector AvoidanceVector = FVector::ZeroVector;
	const int32 TraceCount = FMath::Max(1, LineTraceTimes);
	const float HalfSpread = (TraceCount - 1) * 0.5f;

	for (int32 TraceIndex = 0; TraceIndex < TraceCount; ++TraceIndex)
	{
		const float YawOffset = (TraceIndex - HalfSpread) * AvoidanceTraceYawStepDegrees;
		const FVector TraceDirection = GetActorForwardVector().RotateAngleAxis(YawOffset, FVector::UpVector);
		const FVector TraceStart = GetActorLocation();
		const FVector TraceEnd = TraceStart + TraceDirection * AvoidanceCheckRange;

		FHitResult HitResult;
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CartHorseAvoidance), false, this);
		QueryParams.AddIgnoredActors(ActorsToIgnore);

		const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
		AvoidanceVector += CalculateAvoidence(HitResult.Distance, HitResult.Location, bHit, AvoidanceCheckRange);
	}

	return AvoidanceVector;
}

FVector ACartHorseBase::CalculateAvoidence(float HitDistance, const FVector& HitLocation, bool bHit, float CheckRange) const
{
	if (!bHit)
	{
		return FVector::ZeroVector;
	}

	const float SafeDistance = FMath::Max(HitDistance, 1.0f);
	const float Weight = FMath::Clamp(CheckRange / SafeDistance, 1.0f, 3.0f);
	return (GetActorLocation() - HitLocation).GetSafeNormal() * Weight;
}

void ACartHorseBase::CartForward()
{
	if (!HorseMovementComponent || !SteeringBehaviourComponent)
	{
		return;
	}

	SteeringBehaviourComponent->AddChaseVector(TargetLocation);
	const FSteeringMoveResult MoveResult = SteeringBehaviourComponent->ReturnMoveResult();
	RightInput = MoveResult.RightInput;
	ForwardInput = MoveResult.ForwardInput;
	HorseMovementComponent->AddHorseInput(RightInput, ForwardInput, DefaultSpeedAcceleration, DefaultYawRotationAcceleration);
}

void ACartHorseBase::Flee()
{
	if (!HorseMovementComponent || !SteeringBehaviourComponent)
	{
		return;
	}

	const FSteeringMoveResult MoveResult = SteeringBehaviourComponent->ReturnMoveResult();
	RightInput = MoveResult.RightInput;
	ForwardInput = MoveResult.ForwardInput;
	HorseMovementComponent->AddHorseInput(RightInput, ForwardInput, DefaultSpeedAcceleration, DefaultYawRotationAcceleration);
}

FVector ACartHorseBase::GetHorseLocation_Implementation()
{
	return GetActorLocation();
}

FRotator ACartHorseBase::GetHorseRotation_Implementation()
{
	return GetActorRotation();
}

void ACartHorseBase::UpdateTargetLocation_Implementation(FVector NewTargetLocation)
{
	TargetLocation = NewTargetLocation;
}

void ACartHorseBase::UpdateAvoidenceMode()
{
	if (!SteeringBehaviourComponent)
	{
		return;
	}

	SteeringBehaviourComponent->AddAwayVector(CheckAvoidence(AvoidanceTraceCount));
}
