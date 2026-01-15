#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IStasisable.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UStasisable : public UInterface
{
	GENERATED_BODY()
};

class IStasisable
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Stasis")
	void EnterStasis(double TimeToStasis);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Stasis")
	void ExitStasis();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Stasis")
	void IsInStasis(bool& bIsInStasis);
};