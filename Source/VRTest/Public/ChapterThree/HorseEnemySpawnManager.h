// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChapterThree/Carriage.h"
#include "HorseEnemySpawnManager.generated.h"


class AChapterThreeManager;
UCLASS()
class VRTEST_API AHorseEnemySpawnManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHorseEnemySpawnManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UPROPERTY()
	TObjectPtr<AChapterThreeManager> GlobalManager;
	
	UFUNCTION(BlueprintCallable,BlueprintImplementableEvent,Category="HorseEnemy")
	AActor* GenerateEnemy(USceneComponent* ChasePoint);
};

