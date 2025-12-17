// Fill out your copyright notice in the Description page of Project Settings.


#include "FakePhysicsHandleActor.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"


// Sets default values
AFakePhysicsHandleActor::AFakePhysicsHandleActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AFakePhysicsHandleActor::BeginPlay()
{
	Super::BeginPlay();
	CurrentVelocity = FVector::ZeroVector;
}

// Called every frame
void AFakePhysicsHandleActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Simulating)
	{
		FVector CurrentLocation = GetActorLocation();
		FVector Direction = TargetLocation - CurrentLocation;
		float Length = Direction.Size();
		Direction.Normalize();
		Length = FMath::Clamp(Length, SpringForceMin,SpringForceMax );
		FVector Force = Direction * SpringStiffness * Length - CurrentVelocity * Damping;
		CurrentVelocity += Force * DeltaTime;
		CurrentLocation += CurrentVelocity * DeltaTime;
        
		SetActorLocation(CurrentLocation);
	}

}

void AFakePhysicsHandleActor::SetTargetLocation(const FVector& NewTargetLocation)
{
	TargetLocation = NewTargetLocation;
}

void AFakePhysicsHandleActor::StartSimulate()
{
	Simulating = true;
	CurrentVelocity = FVector::ZeroVector; // 重置速度
}

void AFakePhysicsHandleActor::StopSimulate()
{
	Simulating = false;
	CurrentVelocity = FVector::ZeroVector;
}

