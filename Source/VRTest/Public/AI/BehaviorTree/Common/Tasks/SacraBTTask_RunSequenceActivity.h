// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"

#include "SacraBTTask_RunSequenceActivity.generated.h"

class USacraEnemyActivityComponent;

UCLASS()
class VRTEST_API USacraBTTask_RunSequenceActivity : public UBTTaskNode
{
	GENERATED_BODY()

public:
	USacraBTTask_RunSequenceActivity();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector SequenceActorKey;

private:
	UFUNCTION()
	void HandleSpecialActivityChanged(ESacraEnemySpecialActivityType ActivityType, bool bIsPlaying);

	void FinishTaskWithResult(EBTNodeResult::Type Result);
	void UnbindActivityDelegate();

private:
	UPROPERTY(Transient)
	TObjectPtr<UBehaviorTreeComponent> CachedOwnerComp = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USacraEnemyActivityComponent> CachedActivityComponent = nullptr;
};
