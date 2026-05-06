// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTree/Common/Services/SacraBTService_MeleeAttackTrigger.h"

#include "AIController.h"
#include "AI/Component/SacraEnemyContextComponent.h"
#include "AI/Component/SacraEnemyWeaponComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

USacraBTService_MeleeAttackTrigger::USacraBTService_MeleeAttackTrigger()
{
	NodeName = TEXT("Sacra Melee Attack Trigger");
	bNotifyBecomeRelevant = true;
	bNotifyTick = true;

	FightTargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_MeleeAttackTrigger, FightTargetActorKey), AActor::StaticClass());
	ShouldTriggerMeleeAttackKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_MeleeAttackTrigger, ShouldTriggerMeleeAttackKey));
}

void USacraBTService_MeleeAttackTrigger::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	UpdateTriggerState(OwnerComp);
}

void USacraBTService_MeleeAttackTrigger::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	UpdateTriggerState(OwnerComp);
}

void USacraBTService_MeleeAttackTrigger::UpdateTriggerState(UBehaviorTreeComponent& OwnerComp) const
{
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	APawn* ControlledPawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	if (!BlackboardComponent || !ControlledPawn)
	{
		return;
	}

	bool bShouldTriggerMeleeAttack = false;
	AActor* FightTargetActor = FightTargetActorKey.SelectedKeyName.IsNone()
		? nullptr
		: Cast<AActor>(BlackboardComponent->GetValueAsObject(FightTargetActorKey.SelectedKeyName));

	if (IsValid(FightTargetActor))
	{
		const float DistanceToTarget = FVector::Dist(ControlledPawn->GetActorLocation(), FightTargetActor->GetActorLocation());
		const bool bWithinTriggerDistance = DistanceToTarget <= TriggerDistance;

		USacraEnemyContextComponent* EnemyContextComponent = ControlledPawn->FindComponentByClass<USacraEnemyContextComponent>();
		USacraEnemyWeaponComponent* WeaponComponent = EnemyContextComponent ? EnemyContextComponent->GetCachedWeaponComponent() : nullptr;
		const bool bIsWeaponRangeValid = !bRequireTargetInWeaponAttackRange
			|| (WeaponComponent && WeaponComponent->CanAttackTarget(FightTargetActor, DistanceToTarget));
		const bool bHasLineOfSight = !bRequireLineOfSight || HasLineOfSightToTarget(*ControlledPawn, *FightTargetActor);

		bShouldTriggerMeleeAttack = bWithinTriggerDistance && bIsWeaponRangeValid && bHasLineOfSight;

		const bool bPreviousTriggerState = ShouldTriggerMeleeAttackKey.SelectedKeyName.IsNone()
			? false
			: BlackboardComponent->GetValueAsBool(ShouldTriggerMeleeAttackKey.SelectedKeyName);
		if (bPreviousTriggerState != bShouldTriggerMeleeAttack)
		{
			UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT MeleeTrigger Owner=%s Target=%s Trigger=%s Distance=%.2f Threshold=%.2f RangeValid=%s LoS=%s"),
				*GetNameSafe(OwnerComp.GetAIOwner()),
				*GetNameSafe(FightTargetActor),
				bShouldTriggerMeleeAttack ? TEXT("true") : TEXT("false"),
				DistanceToTarget,
				TriggerDistance,
				bIsWeaponRangeValid ? TEXT("true") : TEXT("false"),
				bHasLineOfSight ? TEXT("true") : TEXT("false"));
		}
	}

	if (!ShouldTriggerMeleeAttackKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent->SetValueAsBool(ShouldTriggerMeleeAttackKey.SelectedKeyName, bShouldTriggerMeleeAttack);
	}
}

bool USacraBTService_MeleeAttackTrigger::HasLineOfSightToTarget(const APawn& ControlledPawn, const AActor& TargetActor) const
{
	const UWorld* World = ControlledPawn.GetWorld();
	if (!World)
	{
		return false;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SacraMeleeAttackTriggerLineOfSight), false, &ControlledPawn);
	QueryParams.AddIgnoredActor(&TargetActor);

	FHitResult HitResult;
	return !World->LineTraceSingleByChannel(
		HitResult,
		ControlledPawn.GetPawnViewLocation(),
		TargetActor.GetActorLocation(),
		LineOfSightTraceChannel,
		QueryParams);
}
