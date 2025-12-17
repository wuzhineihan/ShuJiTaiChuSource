// Fill out your copyright notice in the Description page of Project Settings.


#include "GrassHideVolume.h"
#include "CLM_Character.h"
#include "Components/BrushComponent.h"
#include "GameFramework/Character.h"

#define PrintStr(String) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, String)

AGrassHideVolume::AGrassHideVolume()
{
	GetBrushComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetBrushComponent()->SetCollisionProfileName("GrassCollisionVolume");
	OnActorBeginOverlap.AddDynamic(this,&AGrassHideVolume::OnBeginOverlap);
	OnActorEndOverlap.AddDynamic(this,&AGrassHideVolume::OnEndOverlap);
}

void AGrassHideVolume::OnBeginOverlap(class AActor* OverlappedActor, class AActor* OtherActor)
{
	ACLM_Character* VRPawnCharacter = Cast<ACLM_Character>(OtherActor);
	if(VRPawnCharacter)
		VRPawnCharacter->bIsInGrass = true;
	
}

void AGrassHideVolume::OnEndOverlap(class AActor* OverlappedActor, class AActor* OtherActor)
{
	ACLM_Character* VRPawnCharacter = Cast<ACLM_Character>(OtherActor);
	if(VRPawnCharacter)
		VRPawnCharacter->bIsInGrass = false;
}
