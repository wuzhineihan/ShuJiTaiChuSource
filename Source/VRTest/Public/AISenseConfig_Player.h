// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Perception/AISenseConfig.h"
#include "AISense_Player.h"
#include "AISenseConfig_Player.generated.h"
/**
 * 
 */
UCLASS(meta = (DisplayName = "AI Player config"))
class VRTEST_API UAISenseConfig_Player : public UAISenseConfig
{
	GENERATED_BODY()
public:
	
	/* This is the class that implements the logic for this sense config. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sense", NoClear, config)
	TSubclassOf<UAISense_Player> Implementation;

	/** Maximum sight distance to notice a target. - the radius around the AI that we use to determine if the player is near by  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sense", config, meta = (UIMin = 0.0, ClampMin = 0.0))
	float PlayerRadius{ 1000.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sense", config, meta = (UIMin = 0.0, ClampMin = 0.0,UIMax = 1.57, ClampMax = 1.57))
	float PlayerDegree{ PI/3 };
	
	//UPROPERTY(EditDefaultsOnly,Category="Sense",config,meta=(UIMin = 0.0, ClampMin = 0.00))
	//TArray<FVector3d> Points_location;
	
	/* This is our initializer. We will pull a few things from the parent class here */
	UAISenseConfig_Player(const FObjectInitializer& ObjectInitializer);

	/* When we implement the config this will be called up */
	virtual TSubclassOf<UAISense> GetSenseImplementation() const override;

#if WITH_GAMEPLAY_DEBUGGER
	virtual void DescribeSelfToGameplayDebugger(const UAIPerceptionComponent* PerceptionComponent, FGameplayDebuggerCategory* DebuggerCategory) const override;
#endif // WITH_GAMEPLAY_DEBUGGER
};
