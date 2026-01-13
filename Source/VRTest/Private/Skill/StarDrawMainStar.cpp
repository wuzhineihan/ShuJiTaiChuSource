// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/StarDrawMainStar.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

AStarDrawMainStar::AStarDrawMainStar()
{
	PrimaryActorTick.bCanEverTick = false;

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SetRootComponent(SphereCollision);
	SphereCollision->SetSphereRadius(8.f);
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(SphereCollision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
