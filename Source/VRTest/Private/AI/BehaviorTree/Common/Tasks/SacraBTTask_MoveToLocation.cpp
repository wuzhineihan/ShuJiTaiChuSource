// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTree/Common/Tasks/SacraBTTask_MoveToLocation.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BrainComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"

USacraBTTask_MoveToLocation::USacraBTTask_MoveToLocation()
{
	NodeName = TEXT("Sacra Move To Location");
	bNotifyTick = true;

	BlackboardKey.AddVectorFilter(this, TEXT("BlackboardKey"));
}

EBTNodeResult::Type USacraBTTask_MoveToLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	FMoveToTaskMemory* TaskMemory = reinterpret_cast<FMoveToTaskMemory*>(NodeMemory);
	if (!AIController || !BlackboardComponent || !ControlledPawn)
	{
		return EBTNodeResult::Failed;
	}

	if (TaskMemory)
	{
		TaskMemory->bIsRotatingBeforeMove = false;
	}

	const FVector TargetLocation = BlackboardComponent->GetValueAsVector(GetSelectedBlackboardKey());
	const FVector CurrentLocation = ControlledPawn->GetActorLocation();
	const float DistanceToTarget2D = FVector::Dist2D(CurrentLocation, TargetLocation);
	if (FVector::DistSquared2D(CurrentLocation, TargetLocation) <= FMath::Square(AcceptableRadius))
	{
		UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT MoveToLocation Owner=%s Result=AlreadyWithinRadius Current=%s Target=%s Distance2D=%.2f Radius=%.2f"),
			*GetNameSafe(AIController),
			*CurrentLocation.ToCompactString(),
			*TargetLocation.ToCompactString(),
			DistanceToTarget2D,
			AcceptableRadius);
		return EBTNodeResult::Succeeded;
	}

	if (ShouldRotateBeforeMove(*ControlledPawn) && !IsRotationReady(*ControlledPawn, TargetLocation))
	{
		if (TaskMemory)
		{
			TaskMemory->bIsRotatingBeforeMove = true;
		}

		UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT MoveToLocation Owner=%s Result=PreRotate Current=%s Target=%s Distance2D=%.2f"),
			*GetNameSafe(AIController),
			*CurrentLocation.ToCompactString(),
			*TargetLocation.ToCompactString(),
			DistanceToTarget2D);
		return EBTNodeResult::InProgress;
	}

	return TryStartMove(OwnerComp, *ControlledPawn, TargetLocation);
}

EBTNodeResult::Type USacraBTTask_MoveToLocation::TryStartMove(UBehaviorTreeComponent& OwnerComp, APawn& ControlledPawn, const FVector& TargetLocation)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	const FVector CurrentLocation = ControlledPawn.GetActorLocation();
	const float DistanceToTarget2D = FVector::Dist2D(CurrentLocation, TargetLocation);

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalLocation(TargetLocation);
	MoveRequest.SetAcceptanceRadius(AcceptableRadius);
	MoveRequest.SetAllowPartialPath(bAllowPartialPath);
	MoveRequest.SetProjectGoalLocation(bProjectGoalLocation);
	MoveRequest.SetReachTestIncludesAgentRadius(false);
	MoveRequest.SetReachTestIncludesGoalRadius(false);
	MoveRequest.SetUsePathfinding(true);

	const FPathFollowingRequestResult MoveResult = AIController->MoveTo(MoveRequest);
	if (MoveResult.Code == EPathFollowingRequestResult::Failed)
	{
		UE_LOG(LogTemp, Warning, TEXT("SacraEnemy BT MoveToLocation Owner=%s Result=Failed Current=%s Target=%s Distance2D=%.2f Radius=%.2f"),
			*GetNameSafe(AIController),
			*CurrentLocation.ToCompactString(),
			*TargetLocation.ToCompactString(),
			DistanceToTarget2D,
			AcceptableRadius);
		return EBTNodeResult::Failed;
	}

	if (MoveResult.Code == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT MoveToLocation Owner=%s Result=AlreadyAtGoal Current=%s Target=%s Distance2D=%.2f Radius=%.2f"),
			*GetNameSafe(AIController),
			*CurrentLocation.ToCompactString(),
			*TargetLocation.ToCompactString(),
			DistanceToTarget2D,
			AcceptableRadius);
		return EBTNodeResult::Succeeded;
	}

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT MoveToLocation Owner=%s Result=InProgress Current=%s Target=%s Distance2D=%.2f Radius=%.2f MoveId=%u"),
		*GetNameSafe(AIController),
		*CurrentLocation.ToCompactString(),
		*TargetLocation.ToCompactString(),
		DistanceToTarget2D,
		AcceptableRadius,
		static_cast<uint32>(MoveResult.MoveId));

	WaitForMessage(OwnerComp, UBrainComponent::AIMessage_MoveFinished, MoveResult.MoveId);
	WaitForMessage(OwnerComp, UBrainComponent::AIMessage_RepathFailed);
	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type USacraBTTask_MoveToLocation::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		AIController->StopMovement();
	}

	return Super::AbortTask(OwnerComp, NodeMemory);
}

void USacraBTTask_MoveToLocation::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	FMoveToTaskMemory* TaskMemory = reinterpret_cast<FMoveToTaskMemory*>(NodeMemory);
	if (!TaskMemory || !TaskMemory->bIsRotatingBeforeMove)
	{
		return;
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	if (!AIController || !BlackboardComponent || !ControlledPawn)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	const FVector TargetLocation = BlackboardComponent->GetValueAsVector(GetSelectedBlackboardKey());
	if (IsRotationReady(*ControlledPawn, TargetLocation))
	{
		TaskMemory->bIsRotatingBeforeMove = false;
		const EBTNodeResult::Type MoveResult = TryStartMove(OwnerComp, *ControlledPawn, TargetLocation);
		if (MoveResult == EBTNodeResult::Failed)
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		}
		else if (MoveResult == EBTNodeResult::Succeeded)
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
		return;
	}

	UpdateRotationTowardsTarget(*AIController, *ControlledPawn, TargetLocation, DeltaSeconds);
}

void USacraBTTask_MoveToLocation::OnMessage(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, FName Message, int32 RequestID, bool bSuccess)
{
	Super::OnMessage(OwnerComp, NodeMemory, Message, RequestID, bSuccess);

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT MoveToLocation Finished Owner=%s Message=%s RequestID=%d Success=%s"),
		*GetNameSafe(OwnerComp.GetAIOwner()),
		*Message.ToString(),
		RequestID,
		bSuccess ? TEXT("true") : TEXT("false"));

	const bool bMoveFinishedSuccessfully = (Message == UBrainComponent::AIMessage_MoveFinished) && bSuccess;
	FinishLatentTask(OwnerComp, bMoveFinishedSuccessfully ? EBTNodeResult::Succeeded : EBTNodeResult::Failed);
}

bool USacraBTTask_MoveToLocation::ShouldRotateBeforeMove(const APawn& ControlledPawn) const
{
	switch (PreRotateMode)
	{
	case ESacraMoveToPreRotateMode::Always:
		return true;

	case ESacraMoveToPreRotateMode::Never:
		return false;

	case ESacraMoveToPreRotateMode::Auto:
	default:
		break;
	}

	const ACharacter* ControlledCharacter = Cast<ACharacter>(&ControlledPawn);
	const UCharacterMovementComponent* MovementComponent = ControlledCharacter ? ControlledCharacter->GetCharacterMovement() : nullptr;
	return MovementComponent && MovementComponent->bOrientRotationToMovement;
}

bool USacraBTTask_MoveToLocation::IsRotationReady(const APawn& ControlledPawn, const FVector& TargetLocation) const
{
	FVector DirectionToTarget = TargetLocation - ControlledPawn.GetActorLocation();
	DirectionToTarget.Z = 0.0f;
	if (DirectionToTarget.IsNearlyZero())
	{
		return true;
	}

	const float DesiredYaw = DirectionToTarget.Rotation().Yaw;
	const float CurrentYaw = ControlledPawn.GetActorRotation().Yaw;
	return FMath::Abs(FMath::FindDeltaAngleDegrees(CurrentYaw, DesiredYaw)) <= PreRotateYawTolerance;
}

void USacraBTTask_MoveToLocation::UpdateRotationTowardsTarget(AAIController& AIController, APawn& ControlledPawn, const FVector& TargetLocation, float DeltaSeconds) const
{
	FVector DirectionToTarget = TargetLocation - ControlledPawn.GetActorLocation();
	DirectionToTarget.Z = 0.0f;
	if (DirectionToTarget.IsNearlyZero())
	{
		return;
	}

	const FRotator CurrentRotation = ControlledPawn.GetActorRotation();
	const FRotator DesiredRotation = DirectionToTarget.Rotation();

	float RotationSpeed = FallbackPreRotateSpeed;
	if (const ACharacter* ControlledCharacter = Cast<ACharacter>(&ControlledPawn))
	{
		if (const UCharacterMovementComponent* MovementComponent = ControlledCharacter->GetCharacterMovement())
		{
			RotationSpeed = FMath::Max(RotationSpeed, MovementComponent->RotationRate.Yaw);
		}
	}

	const float CurrentYaw = CurrentRotation.Yaw;
	const float DesiredYaw = DesiredRotation.Yaw;
	const float DeltaYaw = FMath::FindDeltaAngleDegrees(CurrentYaw, DesiredYaw);
	const float MaxYawStep = RotationSpeed * DeltaSeconds;
	const float AppliedYawStep = FMath::Clamp(DeltaYaw, -MaxYawStep, MaxYawStep);
	const float NewYaw = CurrentYaw + AppliedYawStep;
	const FRotator NewRotation(0.0f, NewYaw, 0.0f);
	ControlledPawn.SetActorRotation(NewRotation);
	AIController.SetControlRotation(NewRotation);
}
