// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTree/State/Warning/Tasks/SacraBTTask_ClearWarningSearchLocation.h"

#include "AIController.h"
#include "AI/Component/SacraEnemyContextComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "GameFramework/Pawn.h"

USacraBTTask_ClearWarningSearchLocation::USacraBTTask_ClearWarningSearchLocation()
{
	NodeName = TEXT("Sacra Clear Warning Search Location");

	HasSearchLocationKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_ClearWarningSearchLocation, HasSearchLocationKey));
	SearchLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_ClearWarningSearchLocation, SearchLocationKey));
}

EBTNodeResult::Type USacraBTTask_ClearWarningSearchLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	APawn* ControlledPawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	if (!BlackboardComponent || !ControlledPawn)
	{
		return EBTNodeResult::Failed;
	}

	if (USacraEnemyContextComponent* EnemyContextComponent = ControlledPawn->FindComponentByClass<USacraEnemyContextComponent>())
	{
		EnemyContextComponent->ClearCachedWarningSearchLocation();
	}

	if (!HasSearchLocationKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent->SetValueAsBool(HasSearchLocationKey.SelectedKeyName, false);
	}

	if (!SearchLocationKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent->ClearValue(SearchLocationKey.SelectedKeyName);
	}

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT Warning ClearSearchLocation Owner=%s"),
		*GetNameSafe(OwnerComp.GetAIOwner()));

	return EBTNodeResult::Succeeded;
}
