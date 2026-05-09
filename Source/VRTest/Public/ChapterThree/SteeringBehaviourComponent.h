#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SteeringBehaviourComponent.generated.h"

class APawn;

USTRUCT(BlueprintType)
struct FSteeringMoveResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horse|Steering")
	float ForwardInput = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horse|Steering")
	float RightInput = 0.0f;
};

UCLASS(ClassGroup=(ChapterThree), meta=(BlueprintSpawnableComponent))
class VRTEST_API USteeringBehaviourComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USteeringBehaviourComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="Horse|Steering")
	void AddChaseVector(const FVector& TargetLocation);

	UFUNCTION(BlueprintCallable, Category="Horse|Steering")
	void AddAwayVector(const FVector& InAwayVector);

	UFUNCTION(BlueprintPure, Category="Horse|Steering")
	FSteeringMoveResult ReturnMoveResult();

protected:
	void Initial();
	FSteeringMoveResult CalculateInput();
	void CalculateMoveVector();

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category="Horse|Steering")
	TObjectPtr<APawn> MyPawn = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horse|Steering")
	FVector MoveVector = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horse|Steering")
	FVector AwayVector = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horse|Steering")
	FVector ChaseVector = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horse|Steering")
	float MoveForceParameter = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horse|Steering")
	float AvoidForceParameter = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horse|Steering", meta=(ClampMin="0.0", ClampMax="1.0"))
	float RightInputDeadZone = 0.2f;
};
