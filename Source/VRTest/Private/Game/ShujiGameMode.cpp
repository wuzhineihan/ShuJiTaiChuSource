// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ShujiGameMode.h"
#include "IXRTrackingSystem.h"
#include "IHeadMountedDisplay.h"

UClass* AShujiGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	//如果设置了Default Pawn Class则优先使用
	if (DefaultPawnClass)
		return DefaultPawnClass;
	
	if (GEngine && GEngine->XRSystem.IsValid() && GEngine->XRSystem->GetHMDDevice())
	{
		bIsVRMode = GEngine->XRSystem->GetHMDDevice()->IsHMDEnabled();
	}

	if (bIsVRMode)
	{
		if (VRPawnClass)
		{
			return VRPawnClass;
		}
	}
	else
	{
		if (PCPawnClass)
		{
			return PCPawnClass;
		}
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

bool AShujiGameMode::GetIsVRMode()
{
    return bIsVRMode;
}
