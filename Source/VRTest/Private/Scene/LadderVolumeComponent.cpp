// Fill out your copyright notice in the Description page of Project Settings.

#include "Scene/LadderVolumeComponent.h"

FVector ULadderVolumeComponent::GetLadderNormal() const
{
	if (const AActor* OwnerActor = GetOwner())
	{
		return OwnerActor->GetActorForwardVector().GetSafeNormal();
	}

	return GetForwardVector().GetSafeNormal();
}

FVector ULadderVolumeComponent::GetLadderUp() const
{
	if (const AActor* OwnerActor = GetOwner())
	{
		return OwnerActor->GetActorUpVector().GetSafeNormal();
	}

	return GetUpVector().GetSafeNormal();
}
