// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTree/State/Fight/Tasks/SacraBTTask_EquipWeapon.h"

#include "AIController.h"
#include "AI/Component/SacraEnemyContextComponent.h"
#include "AI/Component/SacraEnemyWeaponComponent.h"
#include "Game/MyGameplayTags.h"
#include "GameFramework/Pawn.h"

USacraBTTask_EquipWeapon::USacraBTTask_EquipWeapon()
{
	NodeName = TEXT("Sacra Equip Weapon");
	bCreateNodeInstance = true;
}

EBTNodeResult::Type USacraBTTask_EquipWeapon::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UnregisterEquipFinishedListener();

	APawn* ControlledPawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	if (!ControlledPawn)
	{
		return EBTNodeResult::Failed;
	}

	USacraEnemyContextComponent* EnemyContextComponent = ControlledPawn->FindComponentByClass<USacraEnemyContextComponent>();
	if (!EnemyContextComponent)
	{
		return EBTNodeResult::Failed;
	}

	USacraEnemyWeaponComponent* WeaponComponent = EnemyContextComponent->GetCachedWeaponComponent();
	if (!WeaponComponent)
	{
		return EBTNodeResult::Failed;
	}

	if (WeaponComponent->IsWeaponEquipped())
	{
		UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT Fight EquipWeapon Owner=%s Result=AlreadyEquipped"),
			*GetNameSafe(OwnerComp.GetAIOwner()));
		return EBTNodeResult::Succeeded;
	}

	CachedOwnerComp = &OwnerComp;
	CachedWeaponComponent = WeaponComponent;
	RegisterEquipFinishedListener();

	if (!WeaponComponent->EquipWeapon())
	{
		UE_LOG(LogTemp, Warning, TEXT("SacraEnemy BT Fight EquipWeapon Owner=%s Result=StartFailed"),
			*GetNameSafe(OwnerComp.GetAIOwner()));
		UnregisterEquipFinishedListener();
		CachedOwnerComp = nullptr;
		CachedWeaponComponent = nullptr;
		return EBTNodeResult::Failed;
	}

	if (WeaponComponent->IsWeaponEquipped())
	{
		UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT Fight EquipWeapon Owner=%s Result=ImmediateSuccess"),
			*GetNameSafe(OwnerComp.GetAIOwner()));
		UnregisterEquipFinishedListener();
		CachedOwnerComp = nullptr;
		CachedWeaponComponent = nullptr;
		return EBTNodeResult::Succeeded;
	}

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT Fight EquipWeapon Owner=%s Result=InProgress"),
		*GetNameSafe(OwnerComp.GetAIOwner()));

	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type USacraBTTask_EquipWeapon::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UnregisterEquipFinishedListener();
	CachedOwnerComp = nullptr;
	CachedWeaponComponent = nullptr;
	return Super::AbortTask(OwnerComp, NodeMemory);
}

void USacraBTTask_EquipWeapon::RegisterEquipFinishedListener()
{
	UnregisterEquipFinishedListener();

	if (!CachedOwnerComp || !UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	WeaponEquipFinishedMessageHandle = MessageSubsystem.RegisterListener<FEnemyWeaponEquipFinishedMessage>(
		MyProjectTags::TAG_AI_Message_Weapon_EquipFinished,
		this,
		&USacraBTTask_EquipWeapon::HandleWeaponEquipFinishedMessage);
}

void USacraBTTask_EquipWeapon::UnregisterEquipFinishedListener()
{
	if (WeaponEquipFinishedMessageHandle.IsValid())
	{
		WeaponEquipFinishedMessageHandle.Unregister();
	}
}

void USacraBTTask_EquipWeapon::FinishTaskWithResult(bool bSucceeded)
{
	UnregisterEquipFinishedListener();

	UBehaviorTreeComponent* OwnerComp = CachedOwnerComp.Get();
	CachedOwnerComp = nullptr;
	CachedWeaponComponent = nullptr;

	if (OwnerComp)
	{
		FinishLatentTask(*OwnerComp, bSucceeded ? EBTNodeResult::Succeeded : EBTNodeResult::Failed);
	}
}

void USacraBTTask_EquipWeapon::HandleWeaponEquipFinishedMessage(FGameplayTag Channel, const FEnemyWeaponEquipFinishedMessage& Message)
{
	if (Channel != MyProjectTags::TAG_AI_Message_Weapon_EquipFinished)
	{
		return;
	}

	if (!CachedWeaponComponent || Message.WeaponComponent != CachedWeaponComponent)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT Fight EquipWeapon Finished Owner=%s Success=%s"),
		*GetNameSafe(CachedOwnerComp.Get() ? CachedOwnerComp.Get()->GetAIOwner() : nullptr),
		Message.bSuccess ? TEXT("true") : TEXT("false"));

	FinishTaskWithResult(Message.bSuccess);
}
