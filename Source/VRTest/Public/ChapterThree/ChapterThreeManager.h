// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Carriage.h"
#include "GameFramework/Actor.h"
#include "ChapterThreeManager.generated.h"

class AHorseEnemySpawnManager;
UCLASS()
class VRTEST_API AChapterThreeManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AChapterThreeManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	//生成敌人的逻辑
	UFUNCTION()
	void GenerateHorseEnemy();
	
	//可供调用负责判断当前生成点是否加入
	UFUNCTION()
	void CheckHorseEnemySpawnPoints(AHorseEnemySpawnManager* CurrentSpwanPoint);
	
	//开始并初始化
	UFUNCTION(BlueprintCallable)
	void StartChapterThree();

	UFUNCTION()
	USceneComponent* AssignChasePoint();

	UFUNCTION(BlueprintCallable,BlueprintImplementableEvent)
	void OnChaseOver();
	
	
	UPROPERTY()
	TArray<AHorseEnemySpawnManager*> HorseEnemySpawnPoints;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ACarriage> CurrentCarriage;

	UPROPERTY()
	FTimerHandle GenerateEnemyTimerHandle;

	UPROPERTY(BlueprintReadWrite)
	TArray<AActor*> CurrentEnemy;
	
	UPROPERTY()
	int CurrentHorseEnemyNums = 0;

	UFUNCTION()
	void OnEnemyDeadHandler(AActor* EnemyHorse);
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="ChapterThree",meta=(AllowPrivateAccess=true))
	int MaxHorseEnemyNums = 4;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="ChapterThree",meta=(AllowPrivateAccess=true))
	int initHorseEnemyNums = 2;

	UPROPERTY()
	bool bStart = false;
};
