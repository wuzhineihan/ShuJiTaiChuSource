// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/Component/SacraEnemyHatredComponent.h"

#include "SacraBlackboardComponent.generated.h"

class USacraEnemyHatredComponent;

UCLASS(ClassGroup = AI, meta = (BlueprintSpawnableComponent))
class VRTEST_API USacraBlackboardComponent : public UBlackboardComponent
{
	GENERATED_BODY()

public:
	USacraBlackboardComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// ==================== Public Interface ====================

	// 绑定仇恨组件，并开始自动同步 Blackboard Key。
	UFUNCTION(BlueprintCallable, Category = "AI|Blackboard")
	void InitAutoCollect(USacraEnemyHatredComponent* InHatredComponent);

	// 立即将当前仇恨信息推送到 Blackboard。
	UFUNCTION(BlueprintCallable, Category = "AI|Blackboard")
	void RefreshAutoCollectedKeys();

	UFUNCTION(BlueprintCallable, Category = "AI|Blackboard")
	void SetAutoCollectPaused(bool bInPaused);

	UFUNCTION(BlueprintPure, Category = "AI|Blackboard")
	bool IsAutoCollectPaused() const { return bIsAutoCollectPaused; }

protected:
	// ==================== Config ====================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Blackboard|Keys")
	FName HatredStateKeyName = TEXT("HatredState");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Blackboard|Keys")
	FName HatredValueKeyName = TEXT("HatredValue");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Blackboard|Keys")
	FName HatredPercentKeyName = TEXT("HatredPercent");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Blackboard|Keys")
	FName HasWarningLocationKeyName = TEXT("HasWarningLocation");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Blackboard|Keys")
	FName WarningLocationKeyName = TEXT("WarningLocation");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Blackboard|Keys")
	FName HasFightTargetKeyName = TEXT("HasFightTarget");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Blackboard|Keys")
	FName FightTargetKeyName = TEXT("FightTarget");

private:
	// ==================== Internal Helpers ====================

	void BindHatredDelegates();
	void UnbindHatredDelegates();

	void SyncHatredState();
	void SyncHatredValue();
	void SyncHatredTargets();

	void SetEnumIfKeyExists(const FName& KeyName, uint8 InValue);
	void SetFloatIfKeyExists(const FName& KeyName, float InValue);
	void SetBoolIfKeyExists(const FName& KeyName, bool bInValue);
	void SetVectorIfKeyExists(const FName& KeyName, const FVector& InValue);
	void SetObjectIfKeyExists(const FName& KeyName, UObject* InValue);
	void ClearValueIfKeyExists(const FName& KeyName);

	UFUNCTION()
	void HandleHatredStateChanged(EHatredState NewState);

	UFUNCTION()
	void HandleHatredValueChanged(float NewValue);

private:
	// ==================== Runtime ====================

	UPROPERTY(Transient)
	TObjectPtr<USacraEnemyHatredComponent> CachedHatredComponent = nullptr;

	UPROPERTY(Transient)
	bool bIsAutoCollectPaused = false;
};
