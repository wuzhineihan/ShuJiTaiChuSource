// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CLM_Character.h"
#include "Perception/AISense.h"
#include "AISense_Player.generated.h"

class UAISense_Player; // needed for inherited methods
class UAISenseConfig_Player; // use to avoid circular dependencies
/**
 * 
 */
UCLASS(ClassGroup = AI, config = Game)
class VRTEST_API UAISense_Player : public UAISense
{
	GENERATED_UCLASS_BODY()
	
	struct FDigestedPlayerProperties
	{
		float PlayerRadius;
		float PlayerSightDegree;
		bool bInvisible;
		FVector Last_Target_Location;
        AActor* Target_Actor;
		FDigestedPlayerProperties();
		FDigestedPlayerProperties(const UAISenseConfig_Player& SenseConfig);
	};

	// using an array instead of a map
    TMap<FPerceptionListenerID,FDigestedPlayerProperties> DigestedProperties;
	bool do_once=true;	
protected:
	virtual float Update() override;
	void OnNewListenerImpl(const FPerceptionListener& NewListener);
	void OnListenerUpdateImpl(const FPerceptionListener& UpdatedListener);
	void OnListenerRemovedImpl(const FPerceptionListener& RemovedListener);
	bool CheckTargetInRange(ACLM_Character* InTarget,float& multinum,FPerceptionListener& Listener);
};
