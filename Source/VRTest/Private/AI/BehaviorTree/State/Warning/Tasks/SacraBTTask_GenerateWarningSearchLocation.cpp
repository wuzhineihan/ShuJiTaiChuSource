// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTree/State/Warning/Tasks/SacraBTTask_GenerateWarningSearchLocation.h"

#include "AIController.h"
#include "AI/Component/SacraEnemyContextComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "GameFramework/Pawn.h"
#include "NavigationSystem.h"
#include "Engine/World.h"

USacraBTTask_GenerateWarningSearchLocation::USacraBTTask_GenerateWarningSearchLocation()
{
	NodeName = TEXT("Sacra Generate Warning Search Location");

	HasWarningAnchorLocationKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_GenerateWarningSearchLocation, HasWarningAnchorLocationKey));
	WarningAnchorLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_GenerateWarningSearchLocation, WarningAnchorLocationKey));
	HasSearchLocationKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_GenerateWarningSearchLocation, HasSearchLocationKey));
	SearchLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTTask_GenerateWarningSearchLocation, SearchLocationKey));
}

EBTNodeResult::Type USacraBTTask_GenerateWarningSearchLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	APawn* ControlledPawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	if (!BlackboardComponent || !ControlledPawn || WarningAnchorLocationKey.SelectedKeyName.IsNone())
	{
		return EBTNodeResult::Failed;
	}

	USacraEnemyContextComponent* EnemyContextComponent = ControlledPawn->FindComponentByClass<USacraEnemyContextComponent>();
	if (!EnemyContextComponent)
	{
		return EBTNodeResult::Failed;
	}

	const bool bHasWarningAnchorLocation = !HasWarningAnchorLocationKey.SelectedKeyName.IsNone()
		? BlackboardComponent->GetValueAsBool(HasWarningAnchorLocationKey.SelectedKeyName)
		: !WarningAnchorLocationKey.SelectedKeyName.IsNone();

	if (!bHasWarningAnchorLocation)
	{
		EnemyContextComponent->ClearCachedWarningSearchLocation();

		if (!HasSearchLocationKey.SelectedKeyName.IsNone())
		{
			BlackboardComponent->SetValueAsBool(HasSearchLocationKey.SelectedKeyName, false);
		}

		if (!SearchLocationKey.SelectedKeyName.IsNone())
		{
			BlackboardComponent->ClearValue(SearchLocationKey.SelectedKeyName);
		}

		UE_LOG(LogTemp, Warning, TEXT("SacraEnemy BT Warning GenerateSearchLocation Failed Owner=%s Reason=MissingAnchor"),
			*GetNameSafe(OwnerComp.GetAIOwner()));

		return EBTNodeResult::Failed;
	}

	const FVector WarningAnchorLocation = BlackboardComponent->GetValueAsVector(WarningAnchorLocationKey.SelectedKeyName);
	FVector SearchLocation = FVector::ZeroVector;
	const bool bGenerated = TryBuildSearchLocation(OwnerComp, WarningAnchorLocation, SearchLocation);

	if (bGenerated)
	{
		EnemyContextComponent->SetCachedWarningSearchLocation(SearchLocation);
	}
	else
	{
		EnemyContextComponent->ClearCachedWarningSearchLocation();
	}

	if (!HasSearchLocationKey.SelectedKeyName.IsNone())
	{
		BlackboardComponent->SetValueAsBool(HasSearchLocationKey.SelectedKeyName, bGenerated);
	}

	if (!SearchLocationKey.SelectedKeyName.IsNone())
	{
		if (bGenerated)
		{
			BlackboardComponent->SetValueAsVector(SearchLocationKey.SelectedKeyName, SearchLocation);
		}
		else
		{
			BlackboardComponent->ClearValue(SearchLocationKey.SelectedKeyName);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy BT Warning GenerateSearchLocation Owner=%s Result=%s Anchor=%s SearchLocation=%s"),
		*GetNameSafe(OwnerComp.GetAIOwner()),
		bGenerated ? TEXT("Succeeded") : TEXT("Failed"),
		*WarningAnchorLocation.ToCompactString(),
		bGenerated ? *SearchLocation.ToCompactString() : TEXT("None"));

	return bGenerated ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}

bool USacraBTTask_GenerateWarningSearchLocation::TryBuildSearchLocation(UBehaviorTreeComponent& OwnerComp, const FVector& WarningAnchorLocation, FVector& OutSearchLocation) const
{
	APawn* ControlledPawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	if (!ControlledPawn)
	{
		return false;
	}

	USacraEnemyContextComponent* EnemyContextComponent = ControlledPawn->FindComponentByClass<USacraEnemyContextComponent>();
	UWorld* World = ControlledPawn->GetWorld();
	if (!EnemyContextComponent || !World)
	{
		return false;
	}

	UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	if (!NavigationSystem)
	{
		return false;
	}

	const FVector EnemyLocation = ControlledPawn->GetActorLocation();
	FVector AnchorDirection = WarningAnchorLocation - EnemyLocation;
	if (AnchorDirection.IsNearlyZero())
	{
		AnchorDirection = ControlledPawn->GetActorForwardVector();
	}

	const FVector BaseDirection = AnchorDirection.GetSafeNormal().IsNearlyZero()
		? FVector::ForwardVector
		: AnchorDirection.GetSafeNormal();

	const float SearchRadius = EnemyContextComponent->GetWarningSearchRadius();
	const float ReachableRadius = EnemyContextComponent->GetWarningSearchReachableRadius();
	const int32 SearchPointCount = FMath::Max(1, EnemyContextComponent->GetWarningSearchPointCount());

	for (int32 Index = 0; Index < SearchPointCount; ++Index)
	{
		const FVector SearchDirection = FMath::VRand();
		const FVector FlatSearchDirection = FVector(SearchDirection.X, SearchDirection.Y, 0.0f).GetSafeNormal();
		const FVector EffectiveDirection = FlatSearchDirection.IsNearlyZero() ? BaseDirection : FlatSearchDirection;
		const float DistanceAlpha = FMath::FRandRange(0.15f, 1.0f);
		const FVector SampleOrigin = WarningAnchorLocation + (EffectiveDirection * SearchRadius * DistanceAlpha);

		FNavLocation CandidateLocation;
		if (NavigationSystem->GetRandomReachablePointInRadius(SampleOrigin, ReachableRadius, CandidateLocation))
		{
			if (bRequireLineOfSightToAnchor && !PassesSightCheck(*World, *ControlledPawn, CandidateLocation.Location, WarningAnchorLocation))
			{
				continue;
			}

			OutSearchLocation = CandidateLocation.Location;
			return true;
		}
	}

	return false;
}

bool USacraBTTask_GenerateWarningSearchLocation::PassesSightCheck(const UWorld& World, const APawn& ControlledPawn, const FVector& CandidateLocation, const FVector& WarningAnchorLocation) const
{
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SacraWarningSearchLocationSight), false, &ControlledPawn);

	FHitResult HitResult;
	return !World.LineTraceSingleByChannel(
		HitResult,
		CandidateLocation + FVector(0.0f, 0.0f, 50.0f),
		WarningAnchorLocation + FVector(0.0f, 0.0f, 50.0f),
		SightTraceChannel,
		QueryParams);
}
