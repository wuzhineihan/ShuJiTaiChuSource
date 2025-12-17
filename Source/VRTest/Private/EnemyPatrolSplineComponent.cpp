// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyPatrolSplineComponent.h"

void UEnemyPatrolSplineComponent::UpdatePatrolPoint()
{
	PatrolPointIndex+=1;
	PatrolPointIndex %= GetNumberOfSplinePoints();
}

FVector UEnemyPatrolSplineComponent::GetPatrolPointLocation()
{
	return GetLocationAtSplinePoint(PatrolPointIndex,ESplineCoordinateSpace::World);
}

bool UEnemyPatrolSplineComponent::CheckPatrolPointEnd()
{
	if (PatrolPointIndex == GetNumberOfSplinePoints() - 1)
	{
		return true;
	}
	return false;
}
