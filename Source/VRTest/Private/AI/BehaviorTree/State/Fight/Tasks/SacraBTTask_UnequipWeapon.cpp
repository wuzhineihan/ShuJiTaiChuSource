// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTree/State/Fight/Tasks/SacraBTTask_UnequipWeapon.h"

#include "AIController.h"
#include "AI/Component/SacraEnemyContextComponent.h"
#include "AI/Component/SacraEnemyWeaponComponent.h"
#include "GameFramework/Pawn.h"

USacraBTTask_UnequipWeapon::USacraBTTask_UnequipWeapon()
{
	NodeName = TEXT("Sacra Unequip Weapon");
}

EBTNodeResult::Type USacraBTTask_UnequipWeapon::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
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

	WeaponComponent->UnequipWeapon();
	UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT Fight UnequipWeapon Owner=%s Result=Succeeded"),
		*GetNameSafe(OwnerComp.GetAIOwner()));
	return EBTNodeResult::Succeeded;
}
