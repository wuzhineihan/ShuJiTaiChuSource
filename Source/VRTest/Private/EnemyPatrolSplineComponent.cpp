// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyPatrolSplineComponent.h"

void UEnemyPatrolSplineComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetAttachParent() != nullptr)
	{
		DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	}
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

void UEnemyPatrolSplineComponent::ResetPatrolIndex(int32 InPatrolPointIndex)
{
	PatrolPointIndex = FMath::Max(0, InPatrolPointIndex);
	bPatrolForward = true;
}
