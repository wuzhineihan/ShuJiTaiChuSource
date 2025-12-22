// Fill out your copyright notice in the Description page of Project Settings.


#include "Effect/AutoRecoverComponent.h"
#include "Effect/AliveComponent.h"

// Sets default values for this component's properties
UAutoRecoverComponent::UAutoRecoverComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UAutoRecoverComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (Owner)
	{
		AliveComponent = Owner->FindComponentByClass<UAliveComponent>();
	}
	
}


// Called every frame
void UAutoRecoverComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (AliveComponent)
	{
		AliveComponent->IncreaseHP(RecoverRate * DeltaTime);
	}
}
