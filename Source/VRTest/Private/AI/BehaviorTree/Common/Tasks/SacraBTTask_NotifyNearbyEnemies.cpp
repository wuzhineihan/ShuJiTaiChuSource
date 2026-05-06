// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTree/Common/Tasks/SacraBTTask_NotifyNearbyEnemies.h"

#include "AI/SacraEnemySubsystem.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "Game/Characters/BaseEnemy.h"

USacraBTTask_NotifyNearbyEnemies::USacraBTTask_NotifyNearbyEnemies()
{
	NodeName = TEXT("Sacra Notify Nearby Enemies");

	WarningLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_NotifyNearbyEnemies, WarningLocationKey));
	FightTargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_NotifyNearbyEnemies, FightTargetActorKey), AActor::StaticClass());
}

EBTNodeResult::Type USacraBTTask_NotifyNearbyEnemies::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	ABaseEnemy* OwnerEnemy = AIController ? Cast<ABaseEnemy>(AIController->GetPawn()) : nullptr;
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	USacraEnemySubsystem* EnemySubsystem = USacraEnemySubsystem::Get(&OwnerComp);
	if (!OwnerEnemy || !BlackboardComponent || !EnemySubsystem)
	{
		return EBTNodeResult::Failed;
	}

	FVector AlertLocation = FVector::ZeroVector;
	bool bHasAlertLocation = false;
	if (!WarningLocationKey.SelectedKeyName.IsNone())
	{
		AlertLocation = BlackboardComponent->GetValueAsVector(WarningLocationKey.SelectedKeyName);
		bHasAlertLocation = true;
	}

	if (!bHasAlertLocation && bUseOwnerLocationWhenNoWarningLocation)
	{
		AlertLocation = OwnerEnemy->GetActorLocation();
		bHasAlertLocation = true;
	}

	if (!bHasAlertLocation)
	{
		return EBTNodeResult::Failed;
	}

	FEnemyWarningAlertMessage AlertMessage;
	AlertMessage.InstigatorActor = OwnerEnemy;
	AlertMessage.AlertLocation = AlertLocation;
	if (!FightTargetActorKey.SelectedKeyName.IsNone())
	{
		AlertMessage.FightTargetActor = Cast<AActor>(BlackboardComponent->GetValueAsObject(FightTargetActorKey.SelectedKeyName));
	}

	const int32 AffectedCount = EnemySubsystem->BroadcastWarningAlert(
		OwnerEnemy,
		AlertMessage,
		NotifyRadius,
		bAffectIdle,
		bAffectWarning,
		bAffectFight);

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT NotifyNearbyEnemies Owner=%s Radius=%.2f AlertLocation=%s FightTarget=%s Affected=%d"),
		*GetNameSafe(AIController),
		NotifyRadius,
		*AlertLocation.ToCompactString(),
		*GetNameSafe(AlertMessage.FightTargetActor),
		AffectedCount);

	return EBTNodeResult::Succeeded;
}
