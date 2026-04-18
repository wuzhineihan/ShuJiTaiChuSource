// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/LearnSkillOrb.h"

#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "Game/CollisionConfig.h"
#include "Game/Characters/BasePlayer.h"
#include "Skill/PlayerSkillComponent.h"
#include "Grabber/PlayerGrabHand.h"

ALearnSkillOrb::ALearnSkillOrb()
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

EGrabType ALearnSkillOrb::GetGrabType_Implementation() const
{
	return EGrabType::Custom;
}

UPrimitiveComponent* ALearnSkillOrb::GetGrabPrimitive_Implementation() const
{
	return Sphere;
}

bool ALearnSkillOrb::CanBeGrabbedBy_Implementation(const UPlayerGrabHand* Hand) const
{
	if (bConsumed || !Hand)
	{
		return false;
	}

	return Cast<ABasePlayer>(Hand->GetOwner()) != nullptr;
}

bool ALearnSkillOrb::CanBeGrabbedByGravityGlove_Implementation() const
{
	return false;
}

bool ALearnSkillOrb::SupportsDualHandGrab_Implementation() const
{
	return false;
}

void ALearnSkillOrb::OnGrabbed_Implementation(UPlayerGrabHand* Hand)
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
			if (SkillToLearn != ESkillType::None)
			{
				SkillComp->LearnSkill(SkillToLearn);
			}
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

void ALearnSkillOrb::OnReleased_Implementation(UPlayerGrabHand* Hand)
{
}

void ALearnSkillOrb::OnGrabSelected_Implementation()
{
}

void ALearnSkillOrb::OnGrabDeselected_Implementation()
{
}

