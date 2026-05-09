// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AI/DataAsset/SacraEnemyConfigDataAsset.h"
#include "AI/Component/SacraEnemyHatredComponent.h"

#include "SacraEnemyStatusUIComponent.generated.h"

class UUserWidget;
class UWidgetComponent;
class USacraEnemyStatusWidget;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class VRTEST_API USacraEnemyStatusUIComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USacraEnemyStatusUIComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void InitStatusUIComponent();
	void RefreshStatusUI();

	UFUNCTION(BlueprintCallable, Category = "Enemy UI|Config")
	void ApplyConfigData(const FSacraEnemyStatusUIConfig& ConfigData);

	UFUNCTION(BlueprintCallable, Category = "Enemy UI|Runtime")
	void SetStatusUIPaused(bool bInPaused);

	UFUNCTION(BlueprintPure, Category = "Enemy UI|Runtime")
	bool IsStatusUIPaused() const { return bIsStatusUIPaused; }

	UFUNCTION()
	void HandleHatredStateChanged(EHatredState NewState);

	UFUNCTION()
	void HandleHatredValueChanged(float NewValue);

protected:
	// ==================== Config ====================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy UI|Config")
	TSubclassOf<UUserWidget> StatusWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy UI|Config")
	FVector WidgetRelativeLocation = FVector(0.0f, 0.0f, 110.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy UI|Config")
	FVector2D DrawSize = FVector2D(160.0f, 48.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy UI|Config")
	bool bHideWhenIdle = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy UI|Config")
	float MaxVisibleDistance = 3000.0f;

private:
	// ==================== Internal Helpers ====================

	void CreateWidgetComponentIfNeeded();
	void BindHatredDelegates();
	void UnbindHatredDelegates();

	USacraEnemyHatredComponent* ResolveHatredComponent() const;
	float GetHatredPercent() const;
	bool ShouldShowWidget() const;
	void UpdateWidgetVisibility();
	void FaceWidgetToPlayerCamera();

private:
	// ==================== Runtime ====================

	UPROPERTY(Transient)
	TObjectPtr<UWidgetComponent> StatusWidgetComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USacraEnemyHatredComponent> CachedHatredComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USacraEnemyStatusWidget> CachedStatusWidget = nullptr;

	UPROPERTY(Transient)
	bool bIsStatusUIPaused = false;
};
