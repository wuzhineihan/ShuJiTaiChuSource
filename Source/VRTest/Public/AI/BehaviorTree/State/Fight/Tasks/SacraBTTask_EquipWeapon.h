// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayTagContainer.h"
#include "GameFramework/GameplayMessageSubsystem.h"

#include "AI/Component/SacraEnemyWeaponComponent.h"

#include "SacraBTTask_EquipWeapon.generated.h"

UCLASS()
class VRTEST_API USacraBTTask_EquipWeapon : public UBTTaskNode
{
	GENERATED_BODY()

public:
	USacraBTTask_EquipWeapon();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	void RegisterEquipFinishedListener();
	void UnregisterEquipFinishedListener();
	void FinishTaskWithResult(bool bSucceeded);

	void HandleWeaponEquipFinishedMessage(FGameplayTag Channel, const FEnemyWeaponEquipFinishedMessage& Message);

private:
	UPROPERTY(Transient)
	TObjectPtr<UBehaviorTreeComponent> CachedOwnerComp = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USacraEnemyWeaponComponent> CachedWeaponComponent = nullptr;

	FGameplayMessageListenerHandle WeaponEquipFinishedMessageHandle;
};
