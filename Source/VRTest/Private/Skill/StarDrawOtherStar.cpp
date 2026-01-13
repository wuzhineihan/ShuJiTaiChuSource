// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/StarDrawOtherStar.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

static const FName Tag_OtherStars(TEXT("OtherStars"));

AStarDrawOtherStar::AStarDrawOtherStar()
{
	PrimaryActorTick.bCanEverTick = false;

	Tags.Add(Tag_OtherStars);

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SetRootComponent(SphereCollision);
	SphereCollision->SetSphereRadius(8.f);
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollision->SetCollisionResponseToAllChannels(ECR_Overlap);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(SphereCollision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
