// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AI/Component/SacraEnemyHatredComponent.h"

#include "SacraEnemyStatusWidget.generated.h"

class UBorder;
class UProgressBar;
class UTextBlock;
class UVerticalBox;

UCLASS()
class VRTEST_API USacraEnemyStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	USacraEnemyStatusWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Enemy UI")
	void SetHatredState(EHatredState InHatredState);

	UFUNCTION(BlueprintCallable, Category = "Enemy UI")
	void SetHatredPercent(float InHatredPercent);

	UFUNCTION(BlueprintCallable, Category = "Enemy UI")
	void SetWidgetVisible(bool bInVisible);

protected:
	virtual void NativeOnInitialized() override;
	virtual void SynchronizeProperties() override;

protected:
	// ==================== Display Config ====================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy UI|Display")
	FText IdleStateText = FText::FromString(TEXT("IDLE"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy UI|Display")
	FText WarningStateText = FText::FromString(TEXT("WARNING"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy UI|Display")
	FText FightStateText = FText::FromString(TEXT("FIGHT"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy UI|Display")
	FText UnknownStateText = FText::FromString(TEXT("UNKNOWN"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy UI|Display")
	FLinearColor IdleStateColor = FLinearColor(0.45f, 0.75f, 0.95f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy UI|Display")
	FLinearColor WarningStateColor = FLinearColor(0.95f, 0.82f, 0.22f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy UI|Display")
	FLinearColor FightStateColor = FLinearColor(0.95f, 0.26f, 0.22f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy UI|Display")
	FLinearColor UnknownStateColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy UI|Display")
	FLinearColor VisibleBackgroundColor = FLinearColor(0.02f, 0.03f, 0.04f, 0.78f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy UI|Display")
	FLinearColor HiddenBackgroundColor = FLinearColor(0.02f, 0.03f, 0.04f, 0.0f);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Enemy UI")
	TObjectPtr<UBorder> RootBorder = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Enemy UI")
	TObjectPtr<UVerticalBox> RootVerticalBox = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Enemy UI")
	TObjectPtr<UTextBlock> StateTextBlock = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Enemy UI")
	TObjectPtr<UProgressBar> HatredProgressBar = nullptr;

private:
	void RefreshVisuals();
	FText GetStateDisplayText() const;
	FLinearColor GetStateDisplayColor() const;

private:
	UPROPERTY(Transient)
	EHatredState CurrentHatredState = EHatredState::Idle;

	UPROPERTY(Transient)
	float CurrentHatredPercent = 0.0f;

	UPROPERTY(Transient)
	bool bIsWidgetVisible = false;
};
