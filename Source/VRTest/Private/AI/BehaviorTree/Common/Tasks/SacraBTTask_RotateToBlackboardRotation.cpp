// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTree/Common/Tasks/SacraBTTask_RotateToBlackboardRotation.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Rotator.h"

USacraBTTask_RotateToBlackboardRotation::USacraBTTask_RotateToBlackboardRotation()
{
	NodeName = TEXT("Sacra Rotate To Blackboard Rotation");

	BlackboardKey.AddRotatorFilter(this, TEXT("BlackboardKey"));
}

EBTNodeResult::Type USacraBTTask_RotateToBlackboardRotation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	AActor* ControlledActor = AIController ? AIController->GetPawn() : nullptr;
	if (!AIController || !BlackboardComponent || !ControlledActor)
	{
		return EBTNodeResult::Failed;
	}

	const FRotator TargetRotation = BlackboardComponent->GetValueAsRotator(GetSelectedBlackboardKey());
	FRotator NewActorRotation = ControlledActor->GetActorRotation();
	NewActorRotation.Yaw = TargetRotation.Yaw;

	if (FMath::Abs(FRotator::NormalizeAxis(NewActorRotation.Yaw - ControlledActor->GetActorRotation().Yaw)) <= YawTolerance)
	{
		return EBTNodeResult::Succeeded;
	}

	ControlledActor->SetActorRotation(NewActorRotation);
	AIController->SetControlRotation(NewActorRotation);

	return EBTNodeResult::Succeeded;
}
