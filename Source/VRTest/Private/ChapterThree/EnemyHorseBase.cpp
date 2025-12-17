// Fill out your copyright notice in the Description page of Project Settings.


#include "ChapterThree/EnemyHorseBase.h"

// Sets default values
AEnemyHorseBase::AEnemyHorseBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEnemyHorseBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEnemyHorseBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemyHorseBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

