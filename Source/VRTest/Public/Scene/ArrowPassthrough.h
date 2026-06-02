// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ArrowPassthrough.generated.h"

UINTERFACE(MinimalAPI)
class UArrowPassthrough : public UInterface
{
	GENERATED_BODY()
};

class VRTEST_API IArrowPassthrough
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnArrowPassThrough(AActor* Arrow);
};
