// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyPatrolSplineComponent.h"

void UEnemyPatrolSplineComponent::OnRegister()
{
	Super::OnRegister();

	if (SavedPatrolPointLocalLocations.Num() > 0
		&& (GetNumberOfSplinePoints() != SavedPatrolPointLocalLocations.Num() || !bSplineHasBeenEdited))
	{
		RestorePatrolPointsFromCache();
	}
}

#if WITH_EDITOR
void UEnemyPatrolSplineComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	if (bIsRestoringPatrolPoints)
	{
		return;
	}

	const FName PropertyName = PropertyChangedEvent.GetPropertyName();
	const FName MemberPropertyName = PropertyChangedEvent.GetMemberPropertyName();
	const bool bSplineEdited =
		PropertyName == GET_MEMBER_NAME_CHECKED(USplineComponent, SplineCurves)
		|| MemberPropertyName == GET_MEMBER_NAME_CHECKED(USplineComponent, SplineCurves)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(USplineComponent, bSplineHasBeenEdited)
		|| MemberPropertyName == GET_MEMBER_NAME_CHECKED(USplineComponent, bSplineHasBeenEdited);

	if (bSplineEdited)
	{
		BakePatrolPointsFromCurrentSpline();
	}
}
#endif

void UEnemyPatrolSplineComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetAttachParent() != nullptr)
	{
		DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	}
}

void UEnemyPatrolSplineComponent::BakePatrolPointsFromCurrentSpline()
{
	TArray<FVector> CurrentLocalPoints;
	const int32 PointCount = GetNumberOfSplinePoints();
	CurrentLocalPoints.Reserve(PointCount);

	for (int32 PointIndex = 0; PointIndex < PointCount; ++PointIndex)
	{
		CurrentLocalPoints.Add(GetLocationAtSplinePoint(PointIndex, ESplineCoordinateSpace::Local));
	}

	ApplyPatrolPointLocalLocations(CurrentLocalPoints, true);
}

void UEnemyPatrolSplineComponent::RestorePatrolPointsFromCache()
{
	ApplyPatrolPointLocalLocations(SavedPatrolPointLocalLocations, false);
}

void UEnemyPatrolSplineComponent::SetPatrolPointLocalLocations(const TArray<FVector>& InLocalPointLocations)
{
	ApplyPatrolPointLocalLocations(InLocalPointLocations, true);
}

void UEnemyPatrolSplineComponent::SetPatrolPointWorldLocations(const TArray<FVector>& InWorldPointLocations)
{
	TArray<FVector> LocalPoints;
	LocalPoints.Reserve(InWorldPointLocations.Num());

	const FTransform ComponentTransform = GetComponentTransform();
	for (const FVector& WorldPoint : InWorldPointLocations)
	{
		LocalPoints.Add(ComponentTransform.InverseTransformPosition(WorldPoint));
	}

	ApplyPatrolPointLocalLocations(LocalPoints, true);
}

bool UEnemyPatrolSplineComponent::HasPatrolPoints() const
{
	return GetNumberOfSplinePoints() > 0;
}

FVector UEnemyPatrolSplineComponent::GetCurrentPatrolPointLocation() const
{
	if (GetNumberOfSplinePoints() <= 0)
	{
		return FVector::ZeroVector;
	}

	const int32 SafeIndex = FMath::Clamp(PatrolPointIndex, 0, GetNumberOfSplinePoints() - 1);
	return GetLocationAtSplinePoint(SafeIndex, ESplineCoordinateSpace::World);
}

FVector UEnemyPatrolSplineComponent::GetNextPatrolPointLocation(bool& bOutHasNextPoint)
{
	bOutHasNextPoint = false;

	const int32 PointCount = GetNumberOfSplinePoints();
	if (PointCount <= 0)
	{
		return FVector::ZeroVector;
	}

	if (PatrolPointIndex >= PointCount)
	{
		PatrolPointIndex = bLoopPatrol ? 0 : (PointCount - 1);
	}

	if (PatrolPointIndex < 0)
	{
		PatrolPointIndex = 0;
	}

	const FVector NextPoint = GetLocationAtSplinePoint(PatrolPointIndex, ESplineCoordinateSpace::World);
	bOutHasNextPoint = true;

	if (bLoopPatrol)
	{
		PatrolPointIndex = (PatrolPointIndex + 1) % PointCount;
	}
	else if (PointCount > 1)
	{
		if (bPatrolForward)
		{
			if (PatrolPointIndex >= PointCount - 1)
			{
				bPatrolForward = false;
				--PatrolPointIndex;
			}
			else
			{
				++PatrolPointIndex;
			}
		}
		else
		{
			if (PatrolPointIndex <= 0)
			{
				bPatrolForward = true;
				++PatrolPointIndex;
			}
			else
			{
				--PatrolPointIndex;
			}
		}
	}

	return NextPoint;
}

bool UEnemyPatrolSplineComponent::AdvancePatrolPoint()
{
	const int32 PointCount = GetNumberOfSplinePoints();
	if (PointCount <= 0)
	{
		return false;
	}

	if (PatrolPointIndex >= PointCount)
	{
		PatrolPointIndex = bLoopPatrol ? 0 : (PointCount - 1);
	}

	if (PatrolPointIndex < 0)
	{
		PatrolPointIndex = 0;
	}

	if (bLoopPatrol)
	{
		PatrolPointIndex = (PatrolPointIndex + 1) % PointCount;
	}
	else if (PointCount > 1)
	{
		if (bPatrolForward)
		{
			if (PatrolPointIndex >= PointCount - 1)
			{
				bPatrolForward = false;
				--PatrolPointIndex;
			}
			else
			{
				++PatrolPointIndex;
			}
		}
		else
		{
			if (PatrolPointIndex <= 0)
			{
				bPatrolForward = true;
				++PatrolPointIndex;
			}
			else
			{
				--PatrolPointIndex;
			}
		}
	}

	return true;
}

void UEnemyPatrolSplineComponent::ApplyPatrolPointLocalLocations(const TArray<FVector>& InLocalPointLocations, bool bMarkDirty)
{
	SavedPatrolPointLocalLocations = InLocalPointLocations;

	TGuardValue<bool> RestoreGuard(bIsRestoringPatrolPoints, true);

	ClearSplinePoints(false);
	for (const FVector& PointLocation : SavedPatrolPointLocalLocations)
	{
		AddSplinePoint(PointLocation, ESplineCoordinateSpace::Local, false);
	}

	UpdateSpline();

	bSplineHasBeenEdited = SavedPatrolPointLocalLocations.Num() > 0;
	bModifiedByConstructionScript = false;

#if WITH_EDITOR
	if (bMarkDirty)
	{
		Modify();
		MarkPackageDirty();
	}
#endif
}

void UEnemyPatrolSplineComponent::ResetPatrolIndex(int32 InPatrolPointIndex)
{
	PatrolPointIndex = FMath::Max(0, InPatrolPointIndex);
	bPatrolForward = true;
}
