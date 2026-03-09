// Fill out your copyright notice in the Description page of Project Settings.

#include "Scene/ClimbableVolume.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Game/Characters/BasePlayer.h"
#include "Game/CollisionConfig.h"
#include "Grabber/PlayerGrabHand.h"

AClimbableVolume::AClimbableVolume()
{
	PrimaryActorTick.bCanEverTick = false;
	SetActorTickEnabled(false);

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	SetRootComponent(Box);
	Box->SetCollisionProfileName(CP_CLIMBABLE_VOLUME);
	Box->SetGenerateOverlapEvents(false);
	Box->SetCanEverAffectNavigation(false);

	PreviewMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMesh"));
	PreviewMesh->SetupAttachment(Box);
	PreviewMesh->SetCollisionProfileName(CP_NO_COLLISION);
	PreviewMesh->SetGenerateOverlapEvents(false);
	PreviewMesh->SetCanEverAffectNavigation(false);
}

void AClimbableVolume::BeginPlay()
{
	Super::BeginPlay();
}

EGrabType AClimbableVolume::GetGrabType_Implementation() const
{
	return EGrabType::Custom;
}

UPrimitiveComponent* AClimbableVolume::GetGrabPrimitive_Implementation() const
{
	return Box;
}

bool AClimbableVolume::CanBeGrabbedBy_Implementation(const UPlayerGrabHand* Hand) const
{
	if (!Hand)
	{
		return false;
	}

	return Cast<ABasePlayer>(Hand->GetOwner()) != nullptr;
}

bool AClimbableVolume::CanBeGrabbedByGravityGlove_Implementation() const
{
	return false;
}

bool AClimbableVolume::SupportsDualHandGrab_Implementation() const
{
	return true;
}

void AClimbableVolume::OnGrabbed_Implementation(UPlayerGrabHand* Hand)
{
	if (!Hand)
	{
		return;
	}

	if (ABasePlayer* Player = Cast<ABasePlayer>(Hand->GetOwner()))
	{
		Player->RegisterClimbGrip(Hand, this);
	}
}

void AClimbableVolume::OnReleased_Implementation(UPlayerGrabHand* Hand)
{
	if (!Hand)
	{
		return;
	}

	if (ABasePlayer* Player = Cast<ABasePlayer>(Hand->GetOwner()))
	{
		Player->UnregisterClimbGrip(Hand, this);
	}
}

void AClimbableVolume::OnGrabSelected_Implementation()
{
}

void AClimbableVolume::OnGrabDeselected_Implementation()
{
}

