// Fill out your copyright notice in the Description page of Project Settings.

#include "Scene/Boat.h"

#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"

ABoat::ABoat()
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(false);

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);

	MoveSpline = CreateDefaultSubobject<USplineComponent>(TEXT("MoveSpline"));
	MoveSpline->SetupAttachment(Root);
	MoveSpline->SetClosedLoop(false);
}

void ABoat::BeginPlay()
{
	Super::BeginPlay();
}

void ABoat::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bIsMoving || !MoveSpline)
	{
		return;
	}

	TravelDistance = FMath::Min(TravelDistance + MoveSpeed * DeltaSeconds, CachedSplineLength);
	UpdateBoatTransformByDistance(TravelDistance);

	if (TravelDistance >= CachedSplineLength - KINDA_SMALL_NUMBER)
	{
		bIsMoving = false;
		SetActorTickEnabled(false);
	}
}

void ABoat::StartMoveOnce()
{
	if (bMoveTriggered || bIsMoving || !MoveSpline)
	{
		return;
	}

	if (MoveSpeed <= KINDA_SMALL_NUMBER)
	{
		UE_LOG(LogTemp, Warning, TEXT("Boat::StartMoveOnce ignored because MoveSpeed is <= 0."));
		return;
	}

	if (MoveSpline->IsClosedLoop())
	{
		UE_LOG(LogTemp, Warning, TEXT("Boat::StartMoveOnce expects a non-closed spline."));
		return;
	}

	if (MoveSpline->GetNumberOfSplinePoints() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("Boat::StartMoveOnce requires at least 2 spline points."));
		return;
	}

	bMoveTriggered = true;
	bIsMoving = true;
	TravelDistance = 0.0f;
	CachedSplineLength = MoveSpline->GetSplineLength();
	CachedSplineWorldTransform = MoveSpline->GetComponentTransform();

	UpdateBoatTransformByDistance(TravelDistance);
	SetActorTickEnabled(true);
}

void ABoat::UpdateBoatTransformByDistance(float DistanceAlongSpline)
{
	if (!MoveSpline)
	{
		return;
	}

	const FVector LocalLocation = MoveSpline->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::Local);
	const FRotator LocalRotation = MoveSpline->GetRotationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::Local);

	const FVector WorldLocation = CachedSplineWorldTransform.TransformPosition(LocalLocation);
	const FRotator WorldRotation = (CachedSplineWorldTransform.GetRotation() * LocalRotation.Quaternion()).Rotator();

	SetActorLocationAndRotation(WorldLocation, WorldRotation);
}
