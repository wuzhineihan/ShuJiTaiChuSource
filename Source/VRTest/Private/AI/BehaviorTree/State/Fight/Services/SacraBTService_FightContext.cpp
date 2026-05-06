// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTree/State/Fight/Services/SacraBTService_FightContext.h"

#include "AIController.h"
#include "AI/Component/SacraEnemyContextComponent.h"
#include "AI/Component/SacraEnemyHatredComponent.h"
#include "AI/Component/SacraEnemyWeaponComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

USacraBTService_FightContext::USacraBTService_FightContext()
{
	NodeName = TEXT("Sacra Fight Context");
	bNotifyBecomeRelevant = true;
	bNotifyTick = true;

	HasFightTargetKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_FightContext, HasFightTargetKey));
	FightTargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_FightContext, FightTargetActorKey), AActor::StaticClass());
	FightTargetLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_FightContext, FightTargetLocationKey));
	DistanceToFightTargetKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_FightContext, DistanceToFightTargetKey));
	HasWeaponEquippedKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_FightContext, HasWeaponEquippedKey));
	IsAttackingKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_FightContext, IsAttackingKey));
	IsTargetInAttackRangeKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_FightContext, IsTargetInAttackRangeKey));
	HasAttackLineOfSightKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_FightContext, HasAttackLineOfSightKey));
	CanAttackTargetKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_FightContext, CanAttackTargetKey));
	ShouldKeepWeaponEquippedKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_FightContext, ShouldKeepWeaponEquippedKey));
	IsInAttackRecoveryKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_FightContext, IsInAttackRecoveryKey));
	ShouldUseDirectApproachKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_FightContext, ShouldUseDirectApproachKey));
}

void USacraBTService_FightContext::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	if (const AAIController* AIController = OwnerComp.GetAIOwner())
	{
		UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT Enter Fight Owner=%s Pawn=%s"),
			*GetNameSafe(AIController),
			*GetNameSafe(AIController->GetPawn()));
	}

	CollectFightContext(OwnerComp);
}

void USacraBTService_FightContext::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	CollectFightContext(OwnerComp);
}

void USacraBTService_FightContext::CollectFightContext(UBehaviorTreeComponent& OwnerComp) const
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

	USacraEnemyHatredComponent* HatredComponent = EnemyContextComponent->GetCachedHatredComponent();
	USacraEnemyWeaponComponent* WeaponComponent = EnemyContextComponent->GetCachedWeaponComponent();

	AActor* FightTargetActor = HatredComponent ? HatredComponent->GetCurrentFightTargetActor() : nullptr;
	const bool bHasFightTarget = IsValid(FightTargetActor);
	const FVector FightTargetLocation = bHasFightTarget ? FightTargetActor->GetActorLocation() : FVector::ZeroVector;
	const float DistanceToFightTarget = bHasFightTarget
		? FVector::Dist(ControlledPawn->GetActorLocation(), FightTargetLocation)
		: 0.0f;
	const bool bHasWeaponEquipped = WeaponComponent && WeaponComponent->IsWeaponEquipped();
	const bool bIsAttacking = WeaponComponent && WeaponComponent->IsAttacking();
	const bool bIsTargetInAttackRange = WeaponComponent && bHasFightTarget
		? WeaponComponent->IsTargetInAttackRange(FightTargetActor, DistanceToFightTarget)
		: false;
	const bool bHasAttackLineOfSight = bHasFightTarget
		? HasLineOfSightToTarget(*ControlledPawn, *FightTargetActor)
		: false;
	const bool bCanAttackTarget = WeaponComponent && bHasFightTarget
		? WeaponComponent->CanAttackTarget(FightTargetActor, DistanceToFightTarget) && (!bCheckAttackLineOfSight || bHasAttackLineOfSight)
		: false;
	const bool bShouldKeepWeaponEquipped = WeaponComponent && bHasFightTarget
		? WeaponComponent->ShouldKeepWeaponEquipped(DistanceToFightTarget)
		: false;
	const bool bIsInAttackRecovery = WeaponComponent && WeaponComponent->IsInAttackRecovery();
	const bool bShouldUseDirectApproach = WeaponComponent && bHasFightTarget
		? WeaponComponent->ShouldUseDirectApproachToTarget(FightTargetActor, DistanceToFightTarget)
		: false;

	const bool bPreviousHasFightTarget = !HasFightTargetKey.SelectedKeyName.IsNone()
		? BlackboardComponent->GetValueAsBool(HasFightTargetKey.SelectedKeyName)
		: false;
	AActor* PreviousFightTargetActor = !FightTargetActorKey.SelectedKeyName.IsNone()
		? Cast<AActor>(BlackboardComponent->GetValueAsObject(FightTargetActorKey.SelectedKeyName))
		: nullptr;
	const bool bPreviousHasWeaponEquipped = !HasWeaponEquippedKey.SelectedKeyName.IsNone()
		? BlackboardComponent->GetValueAsBool(HasWeaponEquippedKey.SelectedKeyName)
		: false;
	const bool bPreviousIsAttacking = !IsAttackingKey.SelectedKeyName.IsNone()
		? BlackboardComponent->GetValueAsBool(IsAttackingKey.SelectedKeyName)
		: false;

	if (!HasFightTargetKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent->SetValueAsBool(HasFightTargetKey.SelectedKeyName, bHasFightTarget);
	}

	if (!FightTargetActorKey.SelectedKeyName.IsNone())
	{
		if (bHasFightTarget)
		{
			BlackboardComponent->SetValueAsObject(FightTargetActorKey.SelectedKeyName, FightTargetActor);
		}
		else
		{
			BlackboardComponent->ClearValue(FightTargetActorKey.SelectedKeyName);
		}
	}

	if (!FightTargetLocationKey.SelectedKeyName.IsNone())
	{
		if (bHasFightTarget)
		{
			BlackboardComponent->SetValueAsVector(FightTargetLocationKey.SelectedKeyName, FightTargetLocation);
		}
		else
		{
			BlackboardComponent->ClearValue(FightTargetLocationKey.SelectedKeyName);
		}
	}

	if (!DistanceToFightTargetKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent->SetValueAsFloat(DistanceToFightTargetKey.SelectedKeyName, DistanceToFightTarget);
	}

	if (!HasWeaponEquippedKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent->SetValueAsBool(HasWeaponEquippedKey.SelectedKeyName, bHasWeaponEquipped);
	}

	if (!IsAttackingKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent->SetValueAsBool(IsAttackingKey.SelectedKeyName, bIsAttacking);
	}

	if (!IsTargetInAttackRangeKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent->SetValueAsBool(IsTargetInAttackRangeKey.SelectedKeyName, bIsTargetInAttackRange);
	}

	if (!HasAttackLineOfSightKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent->SetValueAsBool(HasAttackLineOfSightKey.SelectedKeyName, bHasAttackLineOfSight);
	}

	if (!CanAttackTargetKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent->SetValueAsBool(CanAttackTargetKey.SelectedKeyName, bCanAttackTarget);
	}

	if (!ShouldKeepWeaponEquippedKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent->SetValueAsBool(ShouldKeepWeaponEquippedKey.SelectedKeyName, bShouldKeepWeaponEquipped);
	}

	if (!IsInAttackRecoveryKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent->SetValueAsBool(IsInAttackRecoveryKey.SelectedKeyName, bIsInAttackRecovery);
	}

	if (!ShouldUseDirectApproachKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent->SetValueAsBool(ShouldUseDirectApproachKey.SelectedKeyName, bShouldUseDirectApproach);
	}

	if (bPreviousHasFightTarget != bHasFightTarget
		|| PreviousFightTargetActor != FightTargetActor
		|| bPreviousHasWeaponEquipped != bHasWeaponEquipped
		|| bPreviousIsAttacking != bIsAttacking)
	{
		UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT Fight Context Owner=%s HasFightTarget=%s->%s FightTarget=%s->%s Distance=%.2f Equipped=%s->%s Attacking=%s->%s InRange=%s LoS=%s CanAttack=%s KeepEquipped=%s Cooldown=%s DirectApproach=%s"),
			*GetNameSafe(OwnerComp.GetAIOwner()),
			bPreviousHasFightTarget ? TEXT("true") : TEXT("false"),
			bHasFightTarget ? TEXT("true") : TEXT("false"),
			*GetNameSafe(PreviousFightTargetActor),
			*GetNameSafe(FightTargetActor),
			DistanceToFightTarget,
			bPreviousHasWeaponEquipped ? TEXT("true") : TEXT("false"),
			bHasWeaponEquipped ? TEXT("true") : TEXT("false"),
			bPreviousIsAttacking ? TEXT("true") : TEXT("false"),
			bIsAttacking ? TEXT("true") : TEXT("false"),
			bIsTargetInAttackRange ? TEXT("true") : TEXT("false"),
			bHasAttackLineOfSight ? TEXT("true") : TEXT("false"),
			bCanAttackTarget ? TEXT("true") : TEXT("false"),
			bShouldKeepWeaponEquipped ? TEXT("true") : TEXT("false"),
			bIsInAttackRecovery ? TEXT("true") : TEXT("false"),
			bShouldUseDirectApproach ? TEXT("true") : TEXT("false"));
	}
}

bool USacraBTService_FightContext::HasLineOfSightToTarget(const APawn& ControlledPawn, const AActor& TargetActor) const
{
	if (!bCheckAttackLineOfSight)
	{
		return true;
	}

	const UWorld* World = ControlledPawn.GetWorld();
	if (!World)
	{
		return false;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SacraFightContextLineOfSight), false, &ControlledPawn);
	QueryParams.AddIgnoredActor(&TargetActor);

	FHitResult HitResult;
	return !World->LineTraceSingleByChannel(
		HitResult,
		ControlledPawn.GetPawnViewLocation(),
		TargetActor.GetActorLocation(),
		AttackLineOfSightTraceChannel,
		QueryParams);
}
