// Fill out your copyright notice in the Description page of Project Settings.


#include "ChapterThree/Carriage.h"

#include "EnemyPatrolSplineComponent.h"
#include "ChapterThree/CartHorseInterface.h"


FName ACarriage::CartName = "Cart";
FName ACarriage::HorseName = "Horse";
FName ACarriage::PatrolSplineName = "PatrolSpline";
// Sets default values
ACarriage::ACarriage()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Horse = CreateDefaultSubobject<UChildActorComponent>(HorseName);
	Cart = CreateDefaultSubobject<UChildActorComponent>(CartName);
	PatrolSpline = CreateDefaultSubobject<UEnemyPatrolSplineComponent>(PatrolSplineName);
	PatrolSpline->SetAbsolute(true,true,true);
	RootComponent = Horse;
	PatrolSpline->SetupAttachment(RootComponent);
	Cart->SetupAttachment(RootComponent);
}



// Called when the game starts or when spawned
void ACarriage::BeginPlay()
{
	Super::BeginPlay();
	HorseActor = Horse->GetChildActor();
	CartActor = Cart->GetChildActor();
	if (HorseActor && HorseActor->GetClass()->ImplementsInterface(UCartHorseInterface::StaticClass()))
	{
		ICartHorseInterface::Execute_UpdateTargetLocation(HorseActor,PatrolSpline->GetPatrolPointLocation());
	}
}

void ACarriage::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	FVector Location = Horse->GetComponentLocation();
	FVector ForwardVector = Horse->GetForwardVector();
	Location.Z -= 90.f;
	Cart->SetWorldLocation(Location - ForwardVector * CartDistance);
	PatrolSpline->SetWorldTransform(this->GetActorTransform());
	
}

// Called every frame
void ACarriage::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bMovable)
	{
		CheckPatrolState();
	}
	UpdateCartPosition(DeltaTime);
}

void ACarriage::UpdateCartPosition(float DeltaTime)
{
		
	if (HorseActor && CartActor && HorseActor->GetClass()->ImplementsInterface(UCartHorseInterface::StaticClass()))
	{
		FVector TargetLocation = ICartHorseInterface::Execute_GetHorseLocation(HorseActor);
		FRotator TargetRotation = ICartHorseInterface::Execute_GetHorseRotation(HorseActor);
		FVector ForwardVector = HorseActor->GetActorForwardVector();
		TargetLocation.Z -= 90.f;
		CartActor->SetActorRotation(FMath::RInterpTo(CartActor->GetActorRotation(),TargetRotation,DeltaTime,CartRotateSpeed));
		CartActor->SetActorLocation(TargetLocation - ForwardVector * CartDistance);
	}
	if (bMovable && bEnableShake)
	{
		const float Time = GetWorld()->GetTimeSeconds();
		float ShakeY = FMath::PerlinNoise1D(Time * CartShakeFrequency * 0.8) * CartShakeAmplitude * 0.5f;
		float ShakeZ = FMath::Sin(Time * CartShakeFrequency) * CartShakeAmplitude;
		CartActor->AddActorLocalOffset(FVector(0.f,ShakeY,ShakeZ));
	}
}

void ACarriage::SetMovable(bool Movable)
{
	bMovable = Movable;
	SetHorseMovable(Movable);
}

void ACarriage::CheckPatrolState()
{
	
	float Distance = (PatrolSpline->GetPatrolPointLocation() - HorseActor->GetActorLocation()).Size();
	
	if (FMath::Abs(Distance) <= PatrolPointRange)
	{
		if (PatrolSpline->CheckPatrolPointEnd())
		{
			ArriveFinalLocation();
			GEngine->AddOnScreenDebugMessage(-1,10,FColor::Red,"Patrol Stopped!");
			return;
		}
		PatrolSpline->UpdatePatrolPoint();
		//GEngine->AddOnScreenDebugMessage(-1,100.f,FColor::Red,"Update");
		if (HorseActor && HorseActor->GetClass()->ImplementsInterface(UCartHorseInterface::StaticClass()))
		{
			ICartHorseInterface::Execute_UpdateTargetLocation(HorseActor,PatrolSpline->GetPatrolPointLocation());
			//GEngine->AddOnScreenDebugMessage(-1,100.f,FColor::Red,"UpdateSuccess");
		}
	}
	
}

FVector ACarriage::GetCurrentCarriageLocation()
{
	return CartActor->GetActorLocation();
}

void ACarriage::ArriveFinalLocation()
{
	SetMovable(false);
	SetHorseMovable(false);
	bArriveFinalLocation = true;
}

