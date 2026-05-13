// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UI/PCActionPromptTypes.h"
#include "PCActionPromptComponent.generated.h"

class ABasePCPlayer;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPCPromptsChanged, const TArray<EPCActionPromptType>&, Prompts);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VRTEST_API UPCActionPromptComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPCActionPromptComponent();

	UPROPERTY(BlueprintAssignable)
	FOnPCPromptsChanged OnPromptsChanged;

	UFUNCTION(BlueprintCallable, Category = "UI|Prompts")
	TArray<EPCActionPromptType> GetCurrentPrompts() const { return CurrentPrompts; }

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void RefreshPrompts();
	bool ArePromptsEqual(const TArray<EPCActionPromptType>& A, const TArray<EPCActionPromptType>& B) const;

	UPROPERTY(Transient)
	TObjectPtr<ABasePCPlayer> OwnerPlayer = nullptr;

	UPROPERTY(Transient)
	TArray<EPCActionPromptType> CurrentPrompts;
};
