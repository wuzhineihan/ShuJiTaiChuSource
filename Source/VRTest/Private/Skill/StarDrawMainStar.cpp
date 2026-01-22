// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/StarDrawMainStar.h"

#include "Components/StaticMeshComponent.h"

AStarDrawMainStar::AStarDrawMainStar()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
