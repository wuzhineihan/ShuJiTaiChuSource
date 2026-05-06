// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"

#include "SacraBTTask_MoveToActor.generated.h"

UENUM(BlueprintType)
enum class ESacraMoveToActorPreRotateMode : uint8
{
	Auto,
	Always,
	Never,
};

UCLASS()
class VRTEST_API USacraBTTask_MoveToActor : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	USacraBTTask_MoveToActor();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnMessage(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, FName Message, int32 RequestID, bool bSuccess) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	struct FMoveToActorTaskMemory
	{
		bool bIsRotatingBeforeMove = false;
	};

	bool ShouldRotateBeforeMove(const APawn& ControlledPawn) const;
	EBTNodeResult::Type TryStartMove(UBehaviorTreeComponent& OwnerComp, APawn& ControlledPawn, AActor& TargetActor);
	bool IsRotationReady(const APawn& ControlledPawn, const FVector& TargetLocation) const;
	void UpdateRotationTowardsTarget(AAIController& AIController, APawn& ControlledPawn, const FVector& TargetLocation, float DeltaSeconds) const;

	virtual uint16 GetInstanceMemorySize() const override
	{
		return sizeof(FMoveToActorTaskMemory);
	}

protected:
	UPROPERTY(EditAnywhere, Category = "Move", meta = (ClampMin = "0.0"))
	float AcceptableRadius = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Move")
	bool bAllowPartialPath = true;

	UPROPERTY(EditAnywhere, Category = "Move|Rotation")
	ESacraMoveToActorPreRotateMode PreRotateMode = ESacraMoveToActorPreRotateMode::Auto;

	UPROPERTY(EditAnywhere, Category = "Move|Rotation", meta = (ClampMin = "0.0"))
	float PreRotateYawTolerance = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Move|Rotation", meta = (ClampMin = "0.0"))
	float FallbackPreRotateSpeed = 360.0f;
};
