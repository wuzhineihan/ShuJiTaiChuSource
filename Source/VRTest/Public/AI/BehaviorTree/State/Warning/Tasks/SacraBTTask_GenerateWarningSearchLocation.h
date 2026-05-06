// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"

#include "SacraBTTask_GenerateWarningSearchLocation.generated.h"

UCLASS()
class VRTEST_API USacraBTTask_GenerateWarningSearchLocation : public UBTTaskNode
{
	GENERATED_BODY()

public:
	USacraBTTask_GenerateWarningSearchLocation();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector HasWarningAnchorLocationKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector WarningAnchorLocationKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector HasSearchLocationKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector SearchLocationKey;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bRequireLineOfSightToAnchor = true;

	UPROPERTY(EditAnywhere, Category = "Config")
	TEnumAsByte<ECollisionChannel> SightTraceChannel = ECC_Visibility;

private:
	bool TryBuildSearchLocation(UBehaviorTreeComponent& OwnerComp, const FVector& WarningAnchorLocation, FVector& OutSearchLocation) const;
	bool PassesSightCheck(const UWorld& World, const APawn& ControlledPawn, const FVector& CandidateLocation, const FVector& WarningAnchorLocation) const;
};
