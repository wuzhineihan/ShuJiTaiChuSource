// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/EnergyOrb.h"

#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "Game/CollisionConfig.h"
#include "Game/Characters/BasePlayer.h"
#include "Skill/PlayerSkillComponent.h"
#include "Grabber/PlayerGrabHand.h"

AEnergyOrb::AEnergyOrb()
{
	PrimaryActorTick.bCanEverTick = false;

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	SetRootComponent(Sphere);
	Sphere->InitSphereRadius(18.0f);
	Sphere->SetCollisionProfileName(CP_ENERGY_ORB);
	Sphere->SetGenerateOverlapEvents(true);
	Sphere->SetCanEverAffectNavigation(false);

	IdleEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("IdleEffect"));
	IdleEffect->SetupAttachment(Sphere);
	IdleEffect->SetAutoActivate(true);
	IdleEffect->SetVisibility(true);

	BurstEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BurstEffect"));
	BurstEffect->SetupAttachment(Sphere);
	BurstEffect->SetAutoActivate(false);
	BurstEffect->SetVisibility(false);
}

EGrabType AEnergyOrb::GetGrabType_Implementation() const
{
	return EGrabType::Custom;
}

UPrimitiveComponent* AEnergyOrb::GetGrabPrimitive_Implementation() const
{
	return Sphere;
}

bool AEnergyOrb::CanBeGrabbedBy_Implementation(const UPlayerGrabHand* Hand) const
{
	if (bConsumed || !Hand)
	{
		return false;
	}

	return Cast<ABasePlayer>(Hand->GetOwner()) != nullptr;
}

bool AEnergyOrb::CanBeGrabbedByGravityGlove_Implementation() const
{
	return false;
}

bool AEnergyOrb::SupportsDualHandGrab_Implementation() const
{
	return false;
}

void AEnergyOrb::OnGrabbed_Implementation(UPlayerGrabHand* Hand)
{
	if (bConsumed)
	{
		return;
	}

	bConsumed = true;

	if (ABasePlayer* Player = Hand ? Cast<ABasePlayer>(Hand->GetOwner()) : nullptr)
	{
		if (UPlayerSkillComponent* SkillComp = Player->PlayerSkillComponent)
		{
			SkillComp->AddEnergy(EnergyRestoreAmount);
		}
	}

	if (IdleEffect)
	{
		IdleEffect->Deactivate();
		IdleEffect->SetVisibility(false);
	}

	if (BurstEffect)
	{
		BurstEffect->SetVisibility(true);
		BurstEffect->Activate(true);
	}

	if (Sphere)
	{
		Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (Hand && Hand->HeldActor == this)
	{
		Hand->ReleaseObject();
	}

	SetLifeSpan(DestroyDelay);
}

void AEnergyOrb::OnReleased_Implementation(UPlayerGrabHand* Hand)
{
}

void AEnergyOrb::OnGrabSelected_Implementation()
{
}

void AEnergyOrb::OnGrabDeselected_Implementation()
{
}


