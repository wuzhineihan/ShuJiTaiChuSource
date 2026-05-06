// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTree/Common/Tasks/SacraBTTask_MoveToActor.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BrainComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"

USacraBTTask_MoveToActor::USacraBTTask_MoveToActor()
{
	NodeName = TEXT("Sacra Move To Actor");
	bNotifyTick = true;

	BlackboardKey.AddObjectFilter(this, TEXT("BlackboardKey"), AActor::StaticClass());
}

EBTNodeResult::Type USacraBTTask_MoveToActor::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	FMoveToActorTaskMemory* TaskMemory = reinterpret_cast<FMoveToActorTaskMemory*>(NodeMemory);
	if (!AIController || !BlackboardComponent || !ControlledPawn)
	{
		return EBTNodeResult::Failed;
	}

	if (TaskMemory)
	{
		TaskMemory->bIsRotatingBeforeMove = false;
	}

	AActor* TargetActor = Cast<AActor>(BlackboardComponent->GetValueAsObject(GetSelectedBlackboardKey()));
	if (!IsValid(TargetActor))
	{
		return EBTNodeResult::Failed;
	}

	const FVector TargetLocation = TargetActor->GetActorLocation();
	const FVector CurrentLocation = ControlledPawn->GetActorLocation();
	const float DistanceToTarget2D = FVector::Dist2D(CurrentLocation, TargetLocation);
	if (DistanceToTarget2D <= AcceptableRadius)
	{
		UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT MoveToActor Owner=%s Result=AlreadyWithinRadius Current=%s TargetActor=%s Target=%s Distance2D=%.2f Radius=%.2f"),
			*GetNameSafe(AIController),
			*CurrentLocation.ToCompactString(),
			*GetNameSafe(TargetActor),
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

		UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT MoveToActor Owner=%s Result=PreRotate Current=%s TargetActor=%s Target=%s Distance2D=%.2f"),
			*GetNameSafe(AIController),
			*CurrentLocation.ToCompactString(),
			*GetNameSafe(TargetActor),
			*TargetLocation.ToCompactString(),
			DistanceToTarget2D);
		return EBTNodeResult::InProgress;
	}

	return TryStartMove(OwnerComp, *ControlledPawn, *TargetActor);
}

EBTNodeResult::Type USacraBTTask_MoveToActor::TryStartMove(UBehaviorTreeComponent& OwnerComp, APawn& ControlledPawn, AActor& TargetActor)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	const FVector CurrentLocation = ControlledPawn.GetActorLocation();
	const FVector TargetLocation = TargetActor.GetActorLocation();
	const float DistanceToTarget2D = FVector::Dist2D(CurrentLocation, TargetLocation);

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(&TargetActor);
	MoveRequest.SetAcceptanceRadius(AcceptableRadius);
	MoveRequest.SetAllowPartialPath(bAllowPartialPath);
	MoveRequest.SetProjectGoalLocation(true);
	MoveRequest.SetReachTestIncludesAgentRadius(false);
	MoveRequest.SetReachTestIncludesGoalRadius(false);
	MoveRequest.SetUsePathfinding(true);

	const FPathFollowingRequestResult MoveResult = AIController->MoveTo(MoveRequest);
	if (MoveResult.Code == EPathFollowingRequestResult::Failed)
	{
		UE_LOG(LogTemp, Warning, TEXT("SacraEnemy BT MoveToActor Owner=%s Result=Failed Current=%s TargetActor=%s Target=%s Distance2D=%.2f Radius=%.2f"),
			*GetNameSafe(AIController),
			*CurrentLocation.ToCompactString(),
			*GetNameSafe(&TargetActor),
			*TargetLocation.ToCompactString(),
			DistanceToTarget2D,
			AcceptableRadius);
		return EBTNodeResult::Failed;
	}

	if (MoveResult.Code == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT MoveToActor Owner=%s Result=AlreadyAtGoal Current=%s TargetActor=%s Target=%s Distance2D=%.2f Radius=%.2f"),
			*GetNameSafe(AIController),
			*CurrentLocation.ToCompactString(),
			*GetNameSafe(&TargetActor),
			*TargetLocation.ToCompactString(),
			DistanceToTarget2D,
			AcceptableRadius);
		return EBTNodeResult::Succeeded;
	}

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT MoveToActor Owner=%s Result=InProgress Current=%s TargetActor=%s Target=%s Distance2D=%.2f Radius=%.2f MoveId=%u"),
		*GetNameSafe(AIController),
		*CurrentLocation.ToCompactString(),
		*GetNameSafe(&TargetActor),
		*TargetLocation.ToCompactString(),
		DistanceToTarget2D,
		AcceptableRadius,
		static_cast<uint32>(MoveResult.MoveId));

	WaitForMessage(OwnerComp, UBrainComponent::AIMessage_MoveFinished, MoveResult.MoveId);
	WaitForMessage(OwnerComp, UBrainComponent::AIMessage_RepathFailed);
	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type USacraBTTask_MoveToActor::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		AIController->StopMovement();
	}

	return Super::AbortTask(OwnerComp, NodeMemory);
}

void USacraBTTask_MoveToActor::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	FMoveToActorTaskMemory* TaskMemory = reinterpret_cast<FMoveToActorTaskMemory*>(NodeMemory);
	if (!TaskMemory || !TaskMemory->bIsRotatingBeforeMove)
	{
		return;
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	AActor* TargetActor = BlackboardComponent ? Cast<AActor>(BlackboardComponent->GetValueAsObject(GetSelectedBlackboardKey())) : nullptr;
	if (!AIController || !BlackboardComponent || !ControlledPawn || !IsValid(TargetActor))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	const FVector TargetLocation = TargetActor->GetActorLocation();
	if (IsRotationReady(*ControlledPawn, TargetLocation))
	{
		TaskMemory->bIsRotatingBeforeMove = false;
		const EBTNodeResult::Type MoveResult = TryStartMove(OwnerComp, *ControlledPawn, *TargetActor);
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

void USacraBTTask_MoveToActor::OnMessage(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, FName Message, int32 RequestID, bool bSuccess)
{
	Super::OnMessage(OwnerComp, NodeMemory, Message, RequestID, bSuccess);

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT MoveToActor Finished Owner=%s Message=%s RequestID=%d Success=%s"),
		*GetNameSafe(OwnerComp.GetAIOwner()),
		*Message.ToString(),
		RequestID,
		bSuccess ? TEXT("true") : TEXT("false"));

	const bool bMoveFinishedSuccessfully = (Message == UBrainComponent::AIMessage_MoveFinished) && bSuccess;
	FinishLatentTask(OwnerComp, bMoveFinishedSuccessfully ? EBTNodeResult::Succeeded : EBTNodeResult::Failed);
}

bool USacraBTTask_MoveToActor::ShouldRotateBeforeMove(const APawn& ControlledPawn) const
{
	switch (PreRotateMode)
	{
	case ESacraMoveToActorPreRotateMode::Always:
		return true;

	case ESacraMoveToActorPreRotateMode::Never:
		return false;

	case ESacraMoveToActorPreRotateMode::Auto:
	default:
		break;
	}

	const ACharacter* ControlledCharacter = Cast<ACharacter>(&ControlledPawn);
	const UCharacterMovementComponent* MovementComponent = ControlledCharacter ? ControlledCharacter->GetCharacterMovement() : nullptr;
	return MovementComponent && MovementComponent->bOrientRotationToMovement;
}

bool USacraBTTask_MoveToActor::IsRotationReady(const APawn& ControlledPawn, const FVector& TargetLocation) const
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

void USacraBTTask_MoveToActor::UpdateRotationTowardsTarget(AAIController& AIController, APawn& ControlledPawn, const FVector& TargetLocation, float DeltaSeconds) const
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
