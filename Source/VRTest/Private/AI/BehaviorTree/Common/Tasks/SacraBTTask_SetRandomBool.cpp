// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTree/Common/Tasks/SacraBTTask_SetRandomBool.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"

USacraBTTask_SetRandomBool::USacraBTTask_SetRandomBool()
{
	NodeName = TEXT("Sacra Set Random Bool");

	BlackboardKey.AddBoolFilter(this, TEXT("BlackboardKey"));
}

EBTNodeResult::Type USacraBTTask_SetRandomBool::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComponent || GetSelectedBlackboardKey().IsNone())
	{
		return EBTNodeResult::Failed;
	}

	const float ClampedProbability = FMath::Clamp(TrueProbability, 0.0f, 1.0f);
	const bool bRandomResult = FMath::FRand() <= ClampedProbability;
	BlackboardComponent->SetValueAsBool(GetSelectedBlackboardKey(), bRandomResult);

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT SetRandomBool Owner=%s Key=%s Probability=%.2f Result=%s"),
		*GetNameSafe(OwnerComp.GetAIOwner()),
		*GetSelectedBlackboardKey().ToString(),
		ClampedProbability,
		bRandomResult ? TEXT("true") : TEXT("false"));

	return EBTNodeResult::Succeeded;
}
