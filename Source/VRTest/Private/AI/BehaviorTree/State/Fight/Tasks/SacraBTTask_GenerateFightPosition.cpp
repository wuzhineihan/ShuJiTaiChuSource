// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTree/State/Fight/Tasks/SacraBTTask_GenerateFightPosition.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "NavigationSystem.h"

USacraBTTask_GenerateFightPosition::USacraBTTask_GenerateFightPosition()
{
	NodeName = TEXT("Sacra Generate Fight Position");

	FightTargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_GenerateFightPosition, FightTargetActorKey), AActor::StaticClass());
	FightTargetLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_GenerateFightPosition, FightTargetLocationKey));
	HasFightPositionKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_GenerateFightPosition, HasFightPositionKey));
	FightPositionKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_GenerateFightPosition, FightPositionKey));
	DesiredMoveSpeedKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_GenerateFightPosition, DesiredMoveSpeedKey));
}

EBTNodeResult::Type USacraBTTask_GenerateFightPosition::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	APawn* ControlledPawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	if (!BlackboardComponent || !ControlledPawn)
	{
		return EBTNodeResult::Failed;
	}

	AActor* TargetActor = nullptr;
	FVector TargetLocation = FVector::ZeroVector;
	if (!ResolveFightTarget(OwnerComp, TargetActor, TargetLocation))
	{
		WriteFailureResult(*BlackboardComponent);
		UE_LOG(LogTemp, Warning, TEXT("SacraEnemy BT Fight GeneratePosition Failed Owner=%s Reason=MissingTarget"),
			*GetNameSafe(OwnerComp.GetAIOwner()));
		return EBTNodeResult::Failed;
	}

	const FVector BaseDirection = BuildBaseDirection(*ControlledPawn, TargetActor, TargetLocation);

	FVector FightPosition = FVector::ZeroVector;
	if (!TryBuildFightPosition(OwnerComp, TargetLocation, BaseDirection, TargetActor, FightPosition))
	{
		WriteFailureResult(*BlackboardComponent);
		UE_LOG(LogTemp, Warning, TEXT("SacraEnemy BT Fight GeneratePosition Failed Owner=%s Reason=NoReachablePosition TargetLocation=%s"),
			*GetNameSafe(OwnerComp.GetAIOwner()),
			*TargetLocation.ToCompactString());
		return EBTNodeResult::Failed;
	}

	WriteSuccessResult(*BlackboardComponent, FightPosition);
	UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT Fight GeneratePosition Owner=%s TargetLocation=%s FightPosition=%s"),
		*GetNameSafe(OwnerComp.GetAIOwner()),
		*TargetLocation.ToCompactString(),
		*FightPosition.ToCompactString());
	return EBTNodeResult::Succeeded;
}

bool USacraBTTask_GenerateFightPosition::ResolveFightTarget(UBehaviorTreeComponent& OwnerComp, AActor*& OutTargetActor, FVector& OutTargetLocation) const
{
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComponent)
	{
		return false;
	}

	OutTargetActor = nullptr;
	OutTargetLocation = FVector::ZeroVector;

	if (!FightTargetActorKey.SelectedKeyName.IsNone())
	{
		OutTargetActor = Cast<AActor>(BlackboardComponent->GetValueAsObject(FightTargetActorKey.SelectedKeyName));
	}

	if (IsValid(OutTargetActor))
	{
		OutTargetLocation = OutTargetActor->GetActorLocation();
		return true;
	}

	if (FightTargetLocationKey.SelectedKeyName.IsNone())
	{
		return false;
	}

	OutTargetLocation = BlackboardComponent->GetValueAsVector(FightTargetLocationKey.SelectedKeyName);
	return !OutTargetLocation.IsNearlyZero();
}

FVector USacraBTTask_GenerateFightPosition::BuildBaseDirection(const APawn& ControlledPawn, const AActor* TargetActor, const FVector& TargetLocation) const
{
	switch (DirectionMode)
	{
	case ESacraFightPositionDirectionMode::TowardOwner:
		return (ControlledPawn.GetActorLocation() - TargetLocation).GetSafeNormal();

	case ESacraFightPositionDirectionMode::TargetForward:
		return IsValid(TargetActor) ? TargetActor->GetActorForwardVector().GetSafeNormal() : FVector::ZeroVector;

	case ESacraFightPositionDirectionMode::TargetBackward:
		return IsValid(TargetActor) ? (-TargetActor->GetActorForwardVector()).GetSafeNormal() : FVector::ZeroVector;

	case ESacraFightPositionDirectionMode::TargetRight:
		return IsValid(TargetActor) ? TargetActor->GetActorRightVector().GetSafeNormal() : FVector::ZeroVector;

	case ESacraFightPositionDirectionMode::TargetLeft:
		return IsValid(TargetActor) ? (-TargetActor->GetActorRightVector()).GetSafeNormal() : FVector::ZeroVector;

	default:
		break;
	}

	return (ControlledPawn.GetActorLocation() - TargetLocation).GetSafeNormal();
}

bool USacraBTTask_GenerateFightPosition::TryBuildFightPosition(UBehaviorTreeComponent& OwnerComp, const FVector& TargetLocation, const FVector& BaseDirection, AActor* TargetActor, FVector& OutFightPosition) const
{
	APawn* ControlledPawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	UWorld* World = ControlledPawn ? ControlledPawn->GetWorld() : nullptr;
	if (!ControlledPawn || !World)
	{
		return false;
	}

	UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	if (!NavigationSystem)
	{
		return false;
	}

	const FVector FallbackDirection = BaseDirection.IsNearlyZero()
		? (ControlledPawn->GetActorLocation() - TargetLocation).GetSafeNormal()
		: BaseDirection.GetSafeNormal();
	const FVector SafeDirection = FallbackDirection.IsNearlyZero() ? ControlledPawn->GetActorForwardVector() : FallbackDirection;

	for (int32 SampleIndex = 0; SampleIndex < FMath::Max(1, MaxSampleCount); ++SampleIndex)
	{
		const float YawOffset = SampleIndex == 0
			? 0.0f
			: FMath::FRandRange(-SampleYawHalfAngle, SampleYawHalfAngle);
		const FVector SampleDirection = FRotator(0.0f, YawOffset, 0.0f).RotateVector(SafeDirection);
		const FVector SampleOrigin = TargetLocation + (SampleDirection * PreferredDistance);

		FNavLocation CandidateLocation;
		if (!NavigationSystem->GetRandomReachablePointInRadius(SampleOrigin, SampleRadius, CandidateLocation))
		{
			continue;
		}

		if (FVector::Dist2D(ControlledPawn->GetActorLocation(), CandidateLocation.Location) < MinDistanceFromOwner)
		{
			continue;
		}

		if (!PassesSightCheck(*World, *ControlledPawn, CandidateLocation.Location, TargetLocation, TargetActor))
		{
			continue;
		}

		OutFightPosition = CandidateLocation.Location;
		return true;
	}

	return false;
}

bool USacraBTTask_GenerateFightPosition::PassesSightCheck(const UWorld& World, const APawn& ControlledPawn, const FVector& CandidateLocation, const FVector& TargetLocation, const AActor* TargetActor) const
{
	if (!bRequireLineOfSightToTarget)
	{
		return true;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SacraFightPositionSightCheck), false, &ControlledPawn);
	if (IsValid(TargetActor))
	{
		QueryParams.AddIgnoredActor(TargetActor);
	}

	FHitResult HitResult;
	return !World.LineTraceSingleByChannel(HitResult, CandidateLocation, TargetLocation, SightTraceChannel, QueryParams);
}

void USacraBTTask_GenerateFightPosition::WriteFailureResult(UBlackboardComponent& BlackboardComponent) const
{
	if (!HasFightPositionKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent.SetValueAsBool(HasFightPositionKey.SelectedKeyName, false);
	}

	if (!FightPositionKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent.ClearValue(FightPositionKey.SelectedKeyName);
	}
}

void USacraBTTask_GenerateFightPosition::WriteSuccessResult(UBlackboardComponent& BlackboardComponent, const FVector& FightPosition) const
{
	if (!HasFightPositionKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent.SetValueAsBool(HasFightPositionKey.SelectedKeyName, true);
	}

	if (!FightPositionKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent.SetValueAsVector(FightPositionKey.SelectedKeyName, FightPosition);
	}

	if (!DesiredMoveSpeedKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent.SetValueAsFloat(DesiredMoveSpeedKey.SelectedKeyName, MoveSpeed);
	}
}
