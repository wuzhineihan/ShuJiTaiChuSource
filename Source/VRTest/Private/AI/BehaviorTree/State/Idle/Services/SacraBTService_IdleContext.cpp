// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTree/State/Idle/Services/SacraBTService_IdleContext.h"

#include "AIController.h"
#include "AI/Component/SacraEnemyContextComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Rotator.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "GameFramework/Pawn.h"

USacraBTService_IdleContext::USacraBTService_IdleContext()
{
	NodeName = TEXT("Sacra Idle Context");
	bNotifyBecomeRelevant = true;
	bNotifyTick = true;

	StandLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_IdleContext, StandLocationKey));
	StandRotationKey.AddRotatorFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_IdleContext, StandRotationKey));
	HasPatrolRouteKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_IdleContext, HasPatrolRouteKey));
	IdleMoveSpeedKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_IdleContext, IdleMoveSpeedKey));
	PatrolMoveSpeedKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_IdleContext, PatrolMoveSpeedKey));
	IsAtStandLocationKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_IdleContext, IsAtStandLocationKey));
	IsFacingStandRotationKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_IdleContext, IsFacingStandRotationKey));
}

void USacraBTService_IdleContext::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	CollectIdleContext(OwnerComp);
}

void USacraBTService_IdleContext::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	CollectIdleContext(OwnerComp);
}

void USacraBTService_IdleContext::CollectIdleContext(UBehaviorTreeComponent& OwnerComp) const
{
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	APawn* ControlledPawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	if (!BlackboardComponent || !ControlledPawn)
	{
		return;
	}

	USacraEnemyContextComponent* EnemyContextComponent = ControlledPawn->FindComponentByClass<USacraEnemyContextComponent>();
	if (!EnemyContextComponent)
	{
		return;
	}

	if (!StandLocationKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent->SetValueAsVector(StandLocationKey.SelectedKeyName, EnemyContextComponent->GetStandLocation());
	}

	if (!StandRotationKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent->SetValueAsRotator(StandRotationKey.SelectedKeyName, EnemyContextComponent->GetStandRotation());
	}

	if (!HasPatrolRouteKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent->SetValueAsBool(HasPatrolRouteKey.SelectedKeyName, EnemyContextComponent->HasPatrolRoute());
	}

	if (!IdleMoveSpeedKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent->SetValueAsFloat(IdleMoveSpeedKey.SelectedKeyName, EnemyContextComponent->GetIdleMoveSpeed());
	}

	if (!PatrolMoveSpeedKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent->SetValueAsFloat(PatrolMoveSpeedKey.SelectedKeyName, EnemyContextComponent->GetPatrolMoveSpeed());
	}

	if (!IsAtStandLocationKey.SelectedKeyName.IsNone())
	{
		const FVector StandLocation = EnemyContextComponent->GetStandLocation();
		const FVector CurrentLocation = ControlledPawn->GetActorLocation();
		const float DistanceToStand2D = FVector::Dist2D(CurrentLocation, StandLocation);
		const bool bIsAtStandLocation = FVector::DistSquared2D(CurrentLocation, StandLocation)
			<= FMath::Square(StandLocationAcceptanceRadius);
		BlackboardComponent->SetValueAsBool(IsAtStandLocationKey.SelectedKeyName, bIsAtStandLocation);
	
	}

	if (!IsFacingStandRotationKey.SelectedKeyName.IsNone())
	{
		const float StandYaw = EnemyContextComponent->GetStandRotation().Yaw;
		const float CurrentYaw = ControlledPawn->GetActorRotation().Yaw;
		const bool bIsFacingStandRotation = FMath::Abs(FMath::FindDeltaAngleDegrees(CurrentYaw, StandYaw)) <= StandRotationYawTolerance;
		BlackboardComponent->SetValueAsBool(IsFacingStandRotationKey.SelectedKeyName, bIsFacingStandRotation);
	}
}
