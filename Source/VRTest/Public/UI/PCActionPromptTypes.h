// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PCActionPromptTypes.generated.h"

UENUM(BlueprintType)
enum class EPCActionPromptType : uint8
{
	StarDraw     UMETA(DisplayName = "StarDraw"),
	ToggleBow    UMETA(DisplayName = "ToggleBow"),
	Vault        UMETA(DisplayName = "Vault"),
	Ignite       UMETA(DisplayName = "Ignite"),
	LeftGrab     UMETA(DisplayName = "LeftGrab"),
	RightGrab    UMETA(DisplayName = "RightGrab"),
	LeftRelease  UMETA(DisplayName = "LeftRelease"),
	RightRelease UMETA(DisplayName = "RightRelease"),
	LeftThrow    UMETA(DisplayName = "LeftThrow"),
	RightThrow   UMETA(DisplayName = "RightThrow"),
	Aim          UMETA(DisplayName = "Aim"),
	StopAim      UMETA(DisplayName = "StopAim"),
	DrawBow      UMETA(DisplayName = "DrawBow"),
	ReleaseBow   UMETA(DisplayName = "ReleaseBow"),
	Crouch       UMETA(DisplayName = "Crouch"),
};
