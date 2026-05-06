// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTree/State/Fight/Services/SacraBTService_BowAimContext.h"

#include "AIController.h"
#include "AI/Component/SacraBowWeaponComponent.h"
#include "AI/Component/SacraEnemyContextComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "GameFramework/Pawn.h"

USacraBTService_BowAimContext::USacraBTService_BowAimContext()
{
	NodeName = TEXT("Sacra Bow Aim Context");
	bNotifyBecomeRelevant = true;
	bNotifyTick = true;

	FightTargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_BowAimContext, FightTargetActorKey), AActor::StaticClass());
	DistanceToFightTargetKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_BowAimContext, DistanceToFightTargetKey));
	HasAttackLineOfSightKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_BowAimContext, HasAttackLineOfSightKey));
	ShouldAimWeaponKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_BowAimContext, ShouldAimWeaponKey));
	IsWeaponAimingKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_BowAimContext, IsWeaponAimingKey));
}

void USacraBTService_BowAimContext::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	CollectBowAimContext(OwnerComp);
}

void USacraBTService_BowAimContext::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	CollectBowAimContext(OwnerComp);
}

void USacraBTService_BowAimContext::CollectBowAimContext(UBehaviorTreeComponent& OwnerComp) const
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
		if (!ShouldAimWeaponKey.SelectedKeyName.IsNone())
		{
			BlackboardComponent->SetValueAsBool(ShouldAimWeaponKey.SelectedKeyName, false);
		}
		if (!IsWeaponAimingKey.SelectedKeyName.IsNone())
		{
			BlackboardComponent->SetValueAsBool(IsWeaponAimingKey.SelectedKeyName, false);
		}
		return;
	}

	USacraBowWeaponComponent* BowWeaponComponent = Cast<USacraBowWeaponComponent>(EnemyContextComponent->GetCachedWeaponComponent());
	if (!BowWeaponComponent)
	{
		if (!ShouldAimWeaponKey.SelectedKeyName.IsNone())
		{
			BlackboardComponent->SetValueAsBool(ShouldAimWeaponKey.SelectedKeyName, false);
		}
		if (!IsWeaponAimingKey.SelectedKeyName.IsNone())
		{
			BlackboardComponent->SetValueAsBool(IsWeaponAimingKey.SelectedKeyName, false);
		}
		return;
	}

	AActor* FightTargetActor = FightTargetActorKey.SelectedKeyName.IsNone()
		? nullptr
		: Cast<AActor>(BlackboardComponent->GetValueAsObject(FightTargetActorKey.SelectedKeyName));
	const float DistanceToFightTarget = DistanceToFightTargetKey.SelectedKeyName.IsNone()
		? 0.0f
		: BlackboardComponent->GetValueAsFloat(DistanceToFightTargetKey.SelectedKeyName);
	const bool bHasAttackLineOfSight = HasAttackLineOfSightKey.SelectedKeyName.IsNone()
		? false
		: BlackboardComponent->GetValueAsBool(HasAttackLineOfSightKey.SelectedKeyName);

	const bool bShouldAimWeapon = BowWeaponComponent->ShouldAimAtTarget(FightTargetActor, DistanceToFightTarget, bHasAttackLineOfSight);
	const bool bWasWeaponAiming = BowWeaponComponent->IsWeaponAiming();

	BowWeaponComponent->SetWeaponAiming(bShouldAimWeapon, FightTargetActor);

	const bool bIsWeaponAiming = BowWeaponComponent->IsWeaponAiming();

	if (!ShouldAimWeaponKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent->SetValueAsBool(ShouldAimWeaponKey.SelectedKeyName, bShouldAimWeapon);
	}

	if (!IsWeaponAimingKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent->SetValueAsBool(IsWeaponAimingKey.SelectedKeyName, bIsWeaponAiming);
	}

	if (bWasWeaponAiming != bIsWeaponAiming)
	{
		UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT BowAim Context Owner=%s Target=%s ShouldAim=%s Aiming=%s Distance=%.2f LoS=%s"),
			*GetNameSafe(OwnerComp.GetAIOwner()),
			*GetNameSafe(FightTargetActor),
			bShouldAimWeapon ? TEXT("true") : TEXT("false"),
			bIsWeaponAiming ? TEXT("true") : TEXT("false"),
			DistanceToFightTarget,
			bHasAttackLineOfSight ? TEXT("true") : TEXT("false"));
	}
}
