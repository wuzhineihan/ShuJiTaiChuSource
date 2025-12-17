// Fill out your copyright notice in the Description page of Project Settings.


#include "ChapterThree/HorseEnemySpawnManager.h"
#include "Kismet/GameplayStatics.h"
#include"ChapterThree/ChapterThreeManager.h"

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
	AActor* manager = UGameplayStatics::GetActorOfClass(GetWorld(), AChapterThreeManager::StaticClass());
	GlobalManager = Cast<AChapterThreeManager>(manager);
	GlobalManager->CheckHorseEnemySpawnPoints(this);
}

// Called every frame
void AHorseEnemySpawnManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


