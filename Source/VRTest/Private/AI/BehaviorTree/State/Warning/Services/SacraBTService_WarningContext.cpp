// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTree/State/Warning/Services/SacraBTService_WarningContext.h"

#include "AIController.h"
#include "AI/Component/SacraEnemyContextComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "GameFramework/Pawn.h"

USacraBTService_WarningContext::USacraBTService_WarningContext()
{
	NodeName = TEXT("Sacra Warning Context");
	bNotifyBecomeRelevant = true;
	bNotifyTick = true;

	HasWarningLocationKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_WarningContext, HasWarningLocationKey));
	WarningLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_WarningContext, WarningLocationKey));
	WarningMoveSpeedKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_WarningContext, WarningMoveSpeedKey));
	DesiredMoveSpeedKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_WarningContext, DesiredMoveSpeedKey));
	WarningAnchorLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_WarningContext, WarningAnchorLocationKey));
	HasWarningAnchorLocationKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_WarningContext, HasWarningAnchorLocationKey));
	HasSearchLocationKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_WarningContext, HasSearchLocationKey));
	SearchLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_WarningContext, SearchLocationKey));
	HasReachedWarningAnchorKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_WarningContext, HasReachedWarningAnchorKey));
}

void USacraBTService_WarningContext::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	if (const AAIController* AIController = OwnerComp.GetAIOwner())
	{
		UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT Enter Warning Owner=%s Pawn=%s"),
			*GetNameSafe(AIController),
			*GetNameSafe(AIController->GetPawn()));
	}

	CollectWarningContext(OwnerComp);
}

void USacraBTService_WarningContext::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	CollectWarningContext(OwnerComp);
}

void USacraBTService_WarningContext::CollectWarningContext(UBehaviorTreeComponent& OwnerComp) const
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
		return;
	}

	if (!WarningMoveSpeedKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent->SetValueAsFloat(WarningMoveSpeedKey.SelectedKeyName, EnemyContextComponent->GetWarningMoveSpeed());
	}

	if (!DesiredMoveSpeedKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent->SetValueAsFloat(DesiredMoveSpeedKey.SelectedKeyName, EnemyContextComponent->GetWarningMoveSpeed());
	}

	if (WarningLocationKey.SelectedKeyName.IsNone())
	{
		return;
	}

	const bool bHasValidWarningLocation = !HasWarningLocationKey.SelectedKeyName.IsNone()
		? BlackboardComponent->GetValueAsBool(HasWarningLocationKey.SelectedKeyName)
		: true;

	if (!bHasValidWarningLocation)
	{
		EnemyContextComponent->ClearCachedWarningAnchorLocation();
		EnemyContextComponent->ClearCachedWarningSearchLocation();

		if (!HasWarningAnchorLocationKey.SelectedKeyName.IsNone())
		{
			BlackboardComponent->SetValueAsBool(HasWarningAnchorLocationKey.SelectedKeyName, false);
		}

		if (!WarningAnchorLocationKey.SelectedKeyName.IsNone())
		{
			BlackboardComponent->ClearValue(WarningAnchorLocationKey.SelectedKeyName);
		}

		if (!HasSearchLocationKey.SelectedKeyName.IsNone())
		{
			BlackboardComponent->SetValueAsBool(HasSearchLocationKey.SelectedKeyName, false);
		}

		if (!SearchLocationKey.SelectedKeyName.IsNone())
		{
			BlackboardComponent->ClearValue(SearchLocationKey.SelectedKeyName);
		}

		if (!HasReachedWarningAnchorKey.SelectedKeyName.IsNone())
		{
			BlackboardComponent->SetValueAsBool(HasReachedWarningAnchorKey.SelectedKeyName, false);
		}

		return;
	}

	const FVector WarningLocation = BlackboardComponent->GetValueAsVector(WarningLocationKey.SelectedKeyName);
	const bool bHasCachedAnchorLocation = EnemyContextComponent->HasCachedWarningAnchorLocation();
	const FVector CachedAnchorLocation = EnemyContextComponent->GetCachedWarningAnchorLocation();
	const bool bWarningLocationChanged = !bHasCachedAnchorLocation
		|| !CachedAnchorLocation.Equals(WarningLocation, 1.0f);

	if (bWarningLocationChanged)
	{
		EnemyContextComponent->SetCachedWarningAnchorLocation(WarningLocation);
		EnemyContextComponent->ClearCachedWarningSearchLocation();

		if (!HasSearchLocationKey.SelectedKeyName.IsNone())
		{
			BlackboardComponent->SetValueAsBool(HasSearchLocationKey.SelectedKeyName, false);
		}

		if (!SearchLocationKey.SelectedKeyName.IsNone())
		{
			BlackboardComponent->ClearValue(SearchLocationKey.SelectedKeyName);
		}

		if (!HasReachedWarningAnchorKey.SelectedKeyName.IsNone())
		{
			BlackboardComponent->SetValueAsBool(HasReachedWarningAnchorKey.SelectedKeyName, false);
		}

		UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT Warning AnchorReset Owner=%s WarningLocation=%s PreviousAnchor=%s"),
			*GetNameSafe(OwnerComp.GetAIOwner()),
			*WarningLocation.ToCompactString(),
			bHasCachedAnchorLocation ? *CachedAnchorLocation.ToCompactString() : TEXT("None"));
	}

	if (!HasWarningAnchorLocationKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent->SetValueAsBool(HasWarningAnchorLocationKey.SelectedKeyName, true);
	}

	if (!WarningAnchorLocationKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent->SetValueAsVector(WarningAnchorLocationKey.SelectedKeyName, EnemyContextComponent->GetCachedWarningAnchorLocation());
	}
}
