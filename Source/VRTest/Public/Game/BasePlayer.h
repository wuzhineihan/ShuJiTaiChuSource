// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/BaseCharacter.h"
#include "Effect/FallDamageComponent.h"
#include "Effect/AutoRecoverComponent.h"
#include "BasePlayer.generated.h"

/**
 * 
 */
UCLASS()
class VRTEST_API ABasePlayer : public ABaseCharacter
{
	GENERATED_BODY()

public:
	ABasePlayer();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UFallDamageComponent* FallDamageComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAutoRecoverComponent* AutoRecoverComponent;
	
};
