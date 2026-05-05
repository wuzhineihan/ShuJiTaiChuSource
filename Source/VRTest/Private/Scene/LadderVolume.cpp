// Fill out your copyright notice in the Description page of Project Settings.

#include "Scene/LadderVolume.h"

#include "Components/BoxComponent.h"

ALadderVolume::ALadderVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	LadderBox = CreateDefaultSubobject<UBoxComponent>(TEXT("LadderBox"));
	SetRootComponent(LadderBox);
	LadderBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	LadderBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	LadderBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	LadderBox->SetGenerateOverlapEvents(true);
}

FVector ALadderVolume::GetLadderNormal() const
{
	return GetActorForwardVector().GetSafeNormal();
}

