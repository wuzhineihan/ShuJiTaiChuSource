// Fill out your copyright notice in the Description page of Project Settings.


#include "ChapterThree/HorseEnemySpawnManager.h"
#include "ChapterThree/CarriageChaseSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "ChapterThree/EnemyHorseBase.h"

// Sets default values
AHorseEnemySpawnManager::AHorseEnemySpawnManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AHorseEnemySpawnManager::BeginPlay()
{
	Super::BeginPlay();

	if (UCarriageChaseSubsystem* ChaseSubsystem = UCarriageChaseSubsystem::Get(this))
	{
		ChaseSubsystem->RegisterSpawnPoint(this);
	}
}

void AHorseEnemySpawnManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UCarriageChaseSubsystem* ChaseSubsystem = UCarriageChaseSubsystem::Get(this))
	{
		ChaseSubsystem->UnregisterSpawnPoint(this);
	}

	Super::EndPlay(EndPlayReason);
}

// Called every frame
void AHorseEnemySpawnManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

AActor* AHorseEnemySpawnManager::GenerateEnemy_Implementation(USceneComponent* ChasePoint)
{
	if (!EnemyHorseClass)
	{
		return nullptr;
	}

	FVector SpawnLocation = GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);
	const FRotator SpawnRotation = GetActorRotation();

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = nullptr;

	AEnemyHorseBase* SpawnedEnemy = GetWorld()->SpawnActor<AEnemyHorseBase>(
		EnemyHorseClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParameters);
	if (!IsValid(SpawnedEnemy))
	{
		return nullptr;
	}

	SpawnedEnemy->SetChasePoint(ChasePoint);
	return SpawnedEnemy;
}
