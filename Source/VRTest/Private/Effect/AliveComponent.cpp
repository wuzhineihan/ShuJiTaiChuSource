// Fill out your copyright notice in the Description page of Project Settings.


#include "Effect/AliveComponent.h"

// Sets default values for this component's properties
UAliveComponent::UAliveComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UAliveComponent::BeginPlay()
{
	Super::BeginPlay();

	HP = MaxHP;
	
}

void UAliveComponent::DecreaseHP(float DecreaseAmount)
{
	if (HP <= 0.0f) return;

	HP -= DecreaseAmount;
	if (HP <= 0.0f)
	{
		HP = 0.0f;
		OnDead.Broadcast();
	}
}

void UAliveComponent::IncreaseHP(float IncreaseAmount)
{
	if (HP <= 0.0f) return; // Usually dead characters don't recover HP automatically, logic can be adjusted if needed

	HP += IncreaseAmount;
	if (HP > MaxHP)
	{
		HP = MaxHP;
	}
}

bool UAliveComponent::SetHP(float TargetHP)
{
	if (TargetHP < 0.0f || TargetHP > MaxHP)
	{
		return false;
	}
	HP = TargetHP;
	if (HP <= 0.0f)
	{
		OnDead.Broadcast();
	}
	return true;
}
