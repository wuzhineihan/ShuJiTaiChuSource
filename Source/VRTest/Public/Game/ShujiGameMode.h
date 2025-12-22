// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShujiGameMode.generated.h"

/**
 * 
 */
UCLASS()
class VRTEST_API AShujiGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Classes")
	TSubclassOf<APawn> VRPawnClass;

	UPROPERTY(EditDefaultsOnly, Category = "Classes")
	TSubclassOf<APawn> PCPawnClass;

	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	bool GetIsVRMode();
private:
	bool bIsVRMode = false;
};
