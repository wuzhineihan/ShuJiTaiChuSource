// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/StarDrawOtherStar.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

#include "Skill/SkillTypes.h"

AStarDrawOtherStar::AStarDrawOtherStar()
{
	PrimaryActorTick.bCanEverTick = false;

	// 统一：lower_snake_case
	Tags.Add(SkillTags::OtherStars);

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SetRootComponent(SphereCollision);
	SphereCollision->SetSphereRadius(2.f);
	SphereCollision->SetCollisionProfileName(FName("Profile_Star_Other"));
	SphereCollision->SetGenerateOverlapEvents(true);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(SphereCollision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
