// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/PCPlayerController.h"
#include "Game/ShujiGameMode.h"
#include "GameFramework/PlayerInput.h"
#include "Blueprint/UserWidget.h"

APCPlayerController::APCPlayerController()
{
}

void APCPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

bool APCPlayerController::InputKey(const FInputKeyParams& Params)
{
	if (Params.Key == EKeys::Escape && Params.Event == IE_Pressed && bIsPlayerInputEnabled)
	{
		TogglePause();
		return true;
	}
	return Super::InputKey(Params);
}

void APCPlayerController::PauseGame()
{
	if (bIsPaused) return;
	bIsPaused = true;

	SetPause(true);

	SetShowMouseCursor(true);
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	SetIgnoreLookInput(true);
	SetIgnoreMoveInput(true);

	TSubclassOf<UUserWidget> WidgetClass = nullptr;
	if (AShujiGameMode* GM = Cast<AShujiGameMode>(GetWorld()->GetAuthGameMode()))
	{
		WidgetClass = GM->PauseMenuWidgetClass;
	}

	if (WidgetClass && !PauseMenuInstance)
	{
		PauseMenuInstance = CreateWidget<UUserWidget>(this, WidgetClass);
		if (PauseMenuInstance)
		{
			PauseMenuInstance->AddToViewport();
		}
	}
}

void APCPlayerController::SetPlayerInputEnabled(bool bEnabled)
{
	bIsPlayerInputEnabled = bEnabled;
}

void APCPlayerController::TogglePause()
{
	if (!bIsPlayerInputEnabled) return;

	if (bIsPaused)
	{
		ResumeGame();
	}
	else
	{
		PauseGame();
	}
}

void APCPlayerController::ResumeGame()
{
	if (!bIsPaused) return;
	bIsPaused = false;

	SetPause(false);

	SetShowMouseCursor(false);
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	SetIgnoreLookInput(false);
	SetIgnoreMoveInput(false);

	if (PauseMenuInstance)
	{
		PauseMenuInstance->RemoveFromParent();
		PauseMenuInstance = nullptr;
	}
}
