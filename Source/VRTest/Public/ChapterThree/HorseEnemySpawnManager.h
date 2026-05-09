// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChapterThree/Carriage.h"
#include "ChapterThree/EnemyHorseBase.h"
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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="HorseEnemy")
	AActor* GenerateEnemy(USceneComponent* ChasePoint);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HorseEnemy")
	TSubclassOf<AEnemyHorseBase> EnemyHorseClass;
};
