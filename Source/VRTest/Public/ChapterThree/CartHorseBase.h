#pragma once

#include "CoreMinimal.h"
#include "ChapterThree/CartHorseInterface.h"
#include "GameFramework/Character.h"
#include "CartHorseBase.generated.h"

class ACarriage;
class UAnimalMovementComponent;
class USteeringBehaviourComponent;

UCLASS()
class VRTEST_API ACartHorseBase : public ACharacter, public ICartHorseInterface
{
	GENERATED_BODY()

public:
	ACartHorseBase();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category="Horse")
	void SetMovable(bool bMovable);

	UFUNCTION(BlueprintPure, Category="Horse")
	bool CanMove() const { return bCanMove; }

	UFUNCTION(BlueprintPure, Category="Horse")
	AActor* GetCart();

	UFUNCTION(BlueprintCallable, Category="Horse")
	FVector CheckAvoidence(int32 LineTraceTimes);

	UFUNCTION(BlueprintCallable, Category="Horse")
	FVector CalculateAvoidence(float HitDistance, const FVector& HitLocation, bool bHit, float CheckRange) const;

	UFUNCTION(BlueprintCallable, Category="Horse")
	void CartForward();

	UFUNCTION(BlueprintCallable, Category="Horse")
	void Flee();

	virtual FVector GetHorseLocation_Implementation() override;
	virtual FRotator GetHorseRotation_Implementation() override;
	virtual void UpdateTargetLocation_Implementation(FVector NewTargetLocation) override;

protected:
	void UpdateAvoidenceMode();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horse", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UAnimalMovementComponent> HorseMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horse", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USteeringBehaviourComponent> SteeringBehaviourComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horse")
	float RightInput = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horse")
	float ForwardInput = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horse")
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horse")
	bool bCanMove = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Horse")
	TObjectPtr<AActor> Cart = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horse|Movement", meta=(ClampMin="0.0"))
	float DefaultMaxSpeed = 592.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horse|Movement", meta=(ClampMin="0.0"))
	float DefaultSpeedAcceleration = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horse|Movement", meta=(ClampMin="0.0"))
	float DefaultYawRotationAcceleration = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horse|Avoidance", meta=(ClampMin="0.01"))
	float AvoidanceCheckInterval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horse|Avoidance", meta=(ClampMin="1"))
	int32 AvoidanceTraceCount = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horse|Avoidance", meta=(ClampMin="0.0"))
	float AvoidanceCheckRange = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horse|Avoidance", meta=(ClampMin="0.0"))
	float AvoidanceTraceYawStepDegrees = 12.0f;

private:
	FTimerHandle AvoidanceTimerHandle;
};
