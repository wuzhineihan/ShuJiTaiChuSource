// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Game/BaseEnemy.h"
#include "AlertEnemyBase.generated.h"

class UEventBusComponent;
/**
 * 
 */
UCLASS()
class AAlertEnemyBase : public ABaseEnemy
{
	GENERATED_BODY()
public:
	AAlertEnemyBase();

	UFUNCTION(BlueprintCallable,Category = "AlertEnemy")
	void CheckEventBusComponent();

	UFUNCTION(BlueprintCallable,Category = "TestFunction")
	void TestBoardcast();

	UFUNCTION(BlueprintCallable,Category = "TestFunction")
	void TestListen();

	protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "EventBus")
	TObjectPtr<UEventBusComponent> EventBusComponent;

private:
	void OnEventReceived(FGameplayTag Tag, UObject* Payload);
	int32 EventCount = 0;
};
