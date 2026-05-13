// Fill out your copyright notice in the Description page of Project Settings.

#include "Scene/WindowVaultVolumeComponent.h"
#include "Game/CollisionConfig.h"

UWindowVaultVolumeComponent::UWindowVaultVolumeComponent()
{
	SetCollisionProfileName(CP_WINDOW_VAULT);
	SetGenerateOverlapEvents(false);

	ComponentTags.AddUnique(FName(TEXT("Interact_WindowVault")));
}

FVector UWindowVaultVolumeComponent::GetVaultForward() const
{
	return bUseYAxisAsForward ? GetRightVector() : GetForwardVector();
}

