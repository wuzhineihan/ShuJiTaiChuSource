// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AliveComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDead);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VRTEST_API UAliveComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAliveComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHP = 100.0f;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnDead OnDead;

private:
	UPROPERTY(VisibleAnywhere, Category = "Health")
	float HP;

public:
	UFUNCTION(BlueprintCallable, Category = "Health")
	void DecreaseHP(float DecreaseAmount);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void IncreaseHP(float IncreaseAmount);

	UFUNCTION(BlueprintCallable, Category = "Health")
	bool SetHP(float TargetHP);

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHP() const { return HP; }
};
