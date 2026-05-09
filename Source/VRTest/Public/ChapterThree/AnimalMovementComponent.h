#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AnimalMovementComponent.generated.h"

class APawn;
class UCharacterMovementComponent;
class UCurveFloat;

UCLASS(ClassGroup=(ChapterThree), meta=(BlueprintSpawnableComponent))
class VRTEST_API UAnimalMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAnimalMovementComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="Horse|Movement")
	void AddHorseInput(float InRightInput, float InForwardInput, float InSpeedAcceleration, float InYawRotationAcceleration);

	UFUNCTION(BlueprintCallable, Category="Horse|Movement")
	void SetHorseSpeedAcceleration(float InSpeedAcceleration);

	UFUNCTION(BlueprintCallable, Category="Horse|Movement")
	void SetHorseYawRotationAcceleration(float InYawRotationAcceleration);

	UFUNCTION(BlueprintCallable, Category="Horse|Movement")
	void SetMaxSpeed(float InMaxSpeed);

	UFUNCTION(BlueprintCallable, Category="Horse|Movement")
	void Stop();

	UFUNCTION(BlueprintPure, Category="Horse|Movement")
	float GetYawRatationRate() const { return CurrentYawRotationRate; }

	UFUNCTION(BlueprintPure, Category="Horse|Movement")
	float GetCurrentMaxSpeed() const { return CurrentMaxSpeed; }

protected:
	void Initial();
	void UpdateMovementInput(float DeltaTime);
	void Debug() const;
	float GetMaxYawRotationRate() const;

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category="Horse|Movement")
	TObjectPtr<APawn> MyPawn = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Horse|Movement")
	TObjectPtr<UCharacterMovementComponent> MyCharacterMovement = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horse|Movement")
	float RightInput = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horse|Movement")
	float ForwardInput = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horse|Movement")
	float TargetMaxSpeed = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horse|Movement")
	float CurrentMaxSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horse|Movement", meta=(ClampMin="0.0"))
	float WalkSpeed = 120.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horse|Movement")
	float TargetYawRotationRate = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horse|Movement")
	float CurrentYawRotationRate = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horse|Movement")
	TObjectPtr<UCurveFloat> YawRotationRateCurve = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horse|Movement")
	bool bShowDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horse|Movement", meta=(ClampMin="0.0"))
	float SpeedAcceleration = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horse|Movement", meta=(ClampMin="0.0"))
	float YawRotationAcceleration = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horse|Movement", meta=(ClampMin="0.0"))
	float MaxSpeed = 592.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horse|Movement", meta=(ClampMin="0.0"))
	float SpeedInterpRate = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horse|Movement", meta=(ClampMin="0.0"))
	float SlowdownInterpRate = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horse|Movement", meta=(ClampMin="0.0"))
	float PivotInterpRate = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horse|Movement", meta=(ClampMin="0.0"))
	float YawInterpRate = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horse|Movement", meta=(ClampMin="0.0"))
	float FallbackMaxYawRotationRate = 90.0f;
};
