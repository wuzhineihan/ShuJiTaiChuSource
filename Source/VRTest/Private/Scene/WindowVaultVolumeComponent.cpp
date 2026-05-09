// Fill out your copyright notice in the Description page of Project Settings.

#include "Scene/WindowVaultVolumeComponent.h"

UWindowVaultVolumeComponent::UWindowVaultVolumeComponent()
{
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	SetCollisionResponseToChannel(ECC_Camera, ECR_Block);
	SetGenerateOverlapEvents(false);

	ComponentTags.AddUnique(FName(TEXT("Interact_WindowVault")));
}

