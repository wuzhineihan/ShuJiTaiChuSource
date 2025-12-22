// Fill out your copyright notice in the Description page of Project Settings.


#include "Effect/FallDamageComponent.h"
#include "GameFramework/Character.h"
#include "Effect/AliveComponent.h"

// Sets default values for this component's properties
UFallDamageComponent::UFallDamageComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UFallDamageComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (Owner)
	{
		AliveComponent = Owner->FindComponentByClass<UAliveComponent>();
		ACharacter* Character = Cast<ACharacter>(Owner);
		if (Character)
		{
			Character->LandedDelegate.AddDynamic(this, &UFallDamageComponent::OnLanded);
		}
	}
	
}

// Called every frame
void UFallDamageComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (AActor* Owner = GetOwner())
	{
		LastFrameZVelocity = Owner->GetVelocity().Z;
	}
}

void UFallDamageComponent::OnLanded(const FHitResult& Hit)
{
	if (!AliveComponent) return;

	// Use LastFrameZVelocity because current velocity might be zeroed upon landing
	float FallVelocity = FMath::Abs(LastFrameZVelocity);
	if (FallVelocity > FallDamageThreshold)
	{
		float Damage = (FallVelocity - FallDamageThreshold) * DamageCoefficient;
		AliveComponent->DecreaseHP(Damage);
	}
}
