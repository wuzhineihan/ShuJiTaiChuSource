// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Engine/EngineTypes.h"

#include "SacraBTTask_GenerateFightPosition.generated.h"

UENUM(BlueprintType)
enum class ESacraFightPositionDirectionMode : uint8
{
	TowardOwner,
	TargetForward,
	TargetBackward,
	TargetRight,
	TargetLeft
};

UCLASS()
class VRTEST_API USacraBTTask_GenerateFightPosition : public UBTTaskNode
{
	GENERATED_BODY()

public:
	USacraBTTask_GenerateFightPosition();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	// ==================== Input Keys ====================

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector FightTargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector FightTargetLocationKey;

	// ==================== Output Keys ====================

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector HasFightPositionKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector FightPositionKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector DesiredMoveSpeedKey;

	// ==================== Config ====================

	UPROPERTY(EditAnywhere, Category = "Config")
	ESacraFightPositionDirectionMode DirectionMode = ESacraFightPositionDirectionMode::TargetForward;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0"))
	float PreferredDistance = 400.0f;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0"))
	float MinDistanceFromOwner = 250.0f;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0"))
	float SampleRadius = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "1"))
	int32 MaxSampleCount = 5;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float SampleYawHalfAngle = 45.0f;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bRequireLineOfSightToTarget = true;

	UPROPERTY(EditAnywhere, Category = "Config")
	TEnumAsByte<ECollisionChannel> SightTraceChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ClampMin = "0.0"))
	float MoveSpeed = 350.0f;

private:
	bool ResolveFightTarget(UBehaviorTreeComponent& OwnerComp, AActor*& OutTargetActor, FVector& OutTargetLocation) const;
	FVector BuildBaseDirection(const APawn& ControlledPawn, const AActor* TargetActor, const FVector& TargetLocation) const;
	bool TryBuildFightPosition(UBehaviorTreeComponent& OwnerComp, const FVector& TargetLocation, const FVector& BaseDirection, AActor* TargetActor, FVector& OutFightPosition) const;
	bool PassesSightCheck(const UWorld& World, const APawn& ControlledPawn, const FVector& CandidateLocation, const FVector& TargetLocation, const AActor* TargetActor) const;
	void WriteFailureResult(UBlackboardComponent& BlackboardComponent) const;
	void WriteSuccessResult(UBlackboardComponent& BlackboardComponent, const FVector& FightPosition) const;
};
