// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTree/State/Fight/Tasks/SacraBTTask_StartWeaponAttack.h"

#include "AIController.h"
#include "AI/Component/SacraEnemyContextComponent.h"
#include "AI/Component/SacraEnemyWeaponComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "Game/MyGameplayTags.h"
#include "GameFramework/Pawn.h"

USacraBTTask_StartWeaponAttack::USacraBTTask_StartWeaponAttack()
{
	NodeName = TEXT("Sacra Start Weapon Attack");
	bCreateNodeInstance = true;

	FightTargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_StartWeaponAttack, FightTargetActorKey), AActor::StaticClass());
}

EBTNodeResult::Type USacraBTTask_StartWeaponAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UnregisterAttackFinishedListener();

	APawn* ControlledPawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!ControlledPawn || !BlackboardComponent || FightTargetActorKey.SelectedKeyName.IsNone())
	{
		return EBTNodeResult::Failed;
	}

	USacraEnemyContextComponent* EnemyContextComponent = ControlledPawn->FindComponentByClass<USacraEnemyContextComponent>();
	if (!EnemyContextComponent)
	{
		return EBTNodeResult::Failed;
	}

	USacraEnemyWeaponComponent* WeaponComponent = EnemyContextComponent->GetCachedWeaponComponent();
	AActor* FightTargetActor = Cast<AActor>(BlackboardComponent->GetValueAsObject(FightTargetActorKey.SelectedKeyName));
	if (!WeaponComponent || !IsValid(FightTargetActor))
	{
		return EBTNodeResult::Failed;
	}

	const float DistanceToFightTarget = FVector::Dist(ControlledPawn->GetActorLocation(), FightTargetActor->GetActorLocation());
	if (!WeaponComponent->CanAttackTarget(FightTargetActor, DistanceToFightTarget))
	{
		UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT Fight StartAttack Owner=%s Result=Failed TargetInvalidForAttack Target=%s Distance=%.2f"),
			*GetNameSafe(OwnerComp.GetAIOwner()),
			*GetNameSafe(FightTargetActor),
			DistanceToFightTarget);
		return EBTNodeResult::Failed;
	}

	if (bRequireWeaponEquipped && !WeaponComponent->IsWeaponEquipped())
	{
		UE_LOG(LogTemp, Warning, TEXT("SacraEnemy BT Fight StartAttack Owner=%s Result=Failed WeaponNotEquipped"),
			*GetNameSafe(OwnerComp.GetAIOwner()));
		return EBTNodeResult::Failed;
	}

	if (WeaponComponent->IsAttacking())
	{
		UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT Fight StartAttack Owner=%s Result=%s"),
			*GetNameSafe(OwnerComp.GetAIOwner()),
			bSucceedIfAlreadyAttacking ? TEXT("AlreadyAttackingSucceeded") : TEXT("AlreadyAttackingFailed"));
		return bSucceedIfAlreadyAttacking ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
	}

	CachedOwnerComp = &OwnerComp;
	CachedWeaponComponent = WeaponComponent;
	RegisterAttackFinishedListener();

	if (!WeaponComponent->StartAttack(FightTargetActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("SacraEnemy BT Fight StartAttack Owner=%s Result=StartFailed Target=%s"),
			*GetNameSafe(OwnerComp.GetAIOwner()),
			*GetNameSafe(FightTargetActor));
		UnregisterAttackFinishedListener();
		CachedOwnerComp = nullptr;
		CachedWeaponComponent = nullptr;
		return EBTNodeResult::Failed;
	}

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT Fight StartAttack Owner=%s Result=InProgress Target=%s"),
		*GetNameSafe(OwnerComp.GetAIOwner()),
		*GetNameSafe(FightTargetActor));

	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type USacraBTTask_StartWeaponAttack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (CachedWeaponComponent && CachedWeaponComponent->IsAttacking())
	{
		UnregisterAttackFinishedListener();
		CachedWeaponComponent->FinishAttack(false);
	}
	else
	{
		UnregisterAttackFinishedListener();
	}
	CachedOwnerComp = nullptr;
	CachedWeaponComponent = nullptr;

	return Super::AbortTask(OwnerComp, NodeMemory);
}

void USacraBTTask_StartWeaponAttack::RegisterAttackFinishedListener()
{
	UnregisterAttackFinishedListener();

	if (!CachedOwnerComp || !UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	WeaponAttackFinishedMessageHandle = MessageSubsystem.RegisterListener<FEnemyWeaponAttackFinishedMessage>(
		MyProjectTags::TAG_AI_Message_Weapon_AttackFinished,
		this,
		&USacraBTTask_StartWeaponAttack::HandleWeaponAttackFinishedMessage);
}

void USacraBTTask_StartWeaponAttack::UnregisterAttackFinishedListener()
{
	if (WeaponAttackFinishedMessageHandle.IsValid())
	{
		WeaponAttackFinishedMessageHandle.Unregister();
	}
}

void USacraBTTask_StartWeaponAttack::FinishTaskWithResult(bool bSucceeded)
{
	UnregisterAttackFinishedListener();

	UBehaviorTreeComponent* OwnerComp = CachedOwnerComp.Get();
	CachedOwnerComp = nullptr;
	CachedWeaponComponent = nullptr;

	if (OwnerComp)
	{
		FinishLatentTask(*OwnerComp, bSucceeded ? EBTNodeResult::Succeeded : EBTNodeResult::Failed);
	}
}

void USacraBTTask_StartWeaponAttack::HandleWeaponAttackFinishedMessage(FGameplayTag Channel, const FEnemyWeaponAttackFinishedMessage& Message)
{
	if (Channel != MyProjectTags::TAG_AI_Message_Weapon_AttackFinished)
	{
		return;
	}

	if (!CachedWeaponComponent || Message.WeaponComponent != CachedWeaponComponent)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT Fight StartAttack Finished Owner=%s Success=%s"),
		*GetNameSafe(CachedOwnerComp.Get() ? CachedOwnerComp.Get()->GetAIOwner() : nullptr),
		Message.bSuccess ? TEXT("true") : TEXT("false"));

	FinishTaskWithResult(Message.bSuccess);
}
