// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayTagContainer.h"
#include "GameFramework/GameplayMessageSubsystem.h"

#include "AI/Component/SacraEnemyWeaponComponent.h"

#include "SacraBTTask_StartWeaponAttack.generated.h"

class USacraEnemyWeaponComponent;

UCLASS()
class VRTEST_API USacraBTTask_StartWeaponAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	USacraBTTask_StartWeaponAttack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	// ==================== Input Keys ====================

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector FightTargetActorKey;

	// ==================== Config ====================

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bSucceedIfAlreadyAttacking = true;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bRequireWeaponEquipped = true;

private:
	// ==================== Internal Helpers ====================

	void RegisterAttackFinishedListener();
	void UnregisterAttackFinishedListener();
	void FinishTaskWithResult(bool bSucceeded);

	void HandleWeaponAttackFinishedMessage(FGameplayTag Channel, const FEnemyWeaponAttackFinishedMessage& Message);

private:
	// ==================== Runtime ====================

	UPROPERTY(Transient)
	TObjectPtr<UBehaviorTreeComponent> CachedOwnerComp = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USacraEnemyWeaponComponent> CachedWeaponComponent = nullptr;

	FGameplayMessageListenerHandle WeaponAttackFinishedMessageHandle;
};
