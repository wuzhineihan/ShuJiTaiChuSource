// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/UI/SacraEnemyStatusWidget.h"

#include "Components/Border.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

USacraEnemyStatusWidget::USacraEnemyStatusWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void USacraEnemyStatusWidget::SetHatredState(EHatredState InHatredState)
{
	CurrentHatredState = InHatredState;
	RefreshVisuals();
}

void USacraEnemyStatusWidget::SetHatredPercent(float InHatredPercent)
{
	CurrentHatredPercent = FMath::Clamp(InHatredPercent, 0.0f, 1.0f);
	RefreshVisuals();
}

void USacraEnemyStatusWidget::SetWidgetVisible(bool bInVisible)
{
	bIsWidgetVisible = bInVisible;
	SetVisibility(bIsWidgetVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

void USacraEnemyStatusWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	RefreshVisuals();
	SetWidgetVisible(false);
}

void USacraEnemyStatusWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	RefreshVisuals();
}

void USacraEnemyStatusWidget::RefreshVisuals()
{
	if (!StateTextBlock || !HatredProgressBar || !RootBorder)
	{
		return;
	}

	const FLinearColor StateColor = GetStateDisplayColor();

	StateTextBlock->SetText(GetStateDisplayText());
	StateTextBlock->SetColorAndOpacity(FSlateColor(StateColor));
	HatredProgressBar->SetPercent(CurrentHatredPercent);
	HatredProgressBar->SetFillColorAndOpacity(StateColor);
	RootBorder->SetBrushColor(bIsWidgetVisible ? VisibleBackgroundColor : HiddenBackgroundColor);
}

FText USacraEnemyStatusWidget::GetStateDisplayText() const
{
	switch (CurrentHatredState)
	{
	case EHatredState::Idle:
		return IdleStateText;

	case EHatredState::Warning:
		return WarningStateText;

	case EHatredState::Fight:
		return FightStateText;

	default:
		return UnknownStateText;
	}
}

FLinearColor USacraEnemyStatusWidget::GetStateDisplayColor() const
{
	switch (CurrentHatredState)
	{
	case EHatredState::Idle:
		return IdleStateColor;

	case EHatredState::Warning:
		return WarningStateColor;

	case EHatredState::Fight:
		return FightStateColor;

	default:
		return UnknownStateColor;
	}
}
