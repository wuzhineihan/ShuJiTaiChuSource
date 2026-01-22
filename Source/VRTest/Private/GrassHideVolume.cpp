// Fill out your copyright notice in the Description page of Project Settings.


#include "GrassHideVolume.h"
#include "Components/BrushComponent.h"
#include "Game/BasePlayer.h"
#include "Game/CollisionConfig.h"

#define PrintStr(String) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, String)

AGrassHideVolume::AGrassHideVolume()
{
	GetBrushComponent()->SetCollisionProfileName(CP_GRASS_HIDE);
	GetBrushComponent()->SetGenerateOverlapEvents(true);
	OnActorBeginOverlap.AddDynamic(this,&AGrassHideVolume::OnBeginOverlap);
	OnActorEndOverlap.AddDynamic(this,&AGrassHideVolume::OnEndOverlap);
}

void AGrassHideVolume::OnBeginOverlap(class AActor* OverlappedActor, class AActor* OtherActor)
{
	if (ABasePlayer* Player = Cast<ABasePlayer>(OtherActor))
		Player->SetCameraInGrass(true);
	UE_LOG(LogTemp,Warning,TEXT("Enter Grass"));
}

void AGrassHideVolume::OnEndOverlap(class AActor* OverlappedActor, class AActor* OtherActor)
{
	if (ABasePlayer* Player = Cast<ABasePlayer>(OverlappedActor))
		Player->SetCameraInGrass(false);
}
