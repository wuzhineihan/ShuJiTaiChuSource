// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FakePhysicsHandleActor.generated.h"

UCLASS()
class VRTEST_API AFakePhysicsHandleActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFakePhysicsHandleActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FakePhysicsHandle")
	float SpringForceMin = 0.1f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FakePhysicsHandle")
	float SpringForceMax = 100.0f;
	
	// 目标位置，可在蓝图中设置
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FakePhysicsHandle")
	FVector TargetLocation;
	
	// 弹簧刚度
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FakePhysicsHandle")
	float SpringStiffness = 1000.f;

	// 阻尼系数
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FakePhysicsHandle")
	float Damping = 100.f;

	// 设置目标位置
	UFUNCTION(BlueprintCallable, Category = "FakePhysicsHandle")
	void SetTargetLocation(const FVector& NewTargetLocation);

	UFUNCTION(BlueprintCallable, Category = "FakePhysicsHandle")
	void SetCurrentVelocity(const FVector& NewVelocity)
	{
		CurrentVelocity = NewVelocity;
	}

	UFUNCTION(BlueprintCallable, Category = "FakePhysicsHandle")
	void StartSimulate();
	
	UFUNCTION(BlueprintCallable, Category = "FakePhysicsHandle")
	void StopSimulate();

private:
	// 当前速度
	FVector CurrentVelocity = FVector::ZeroVector;

	bool Simulating = false;
};
