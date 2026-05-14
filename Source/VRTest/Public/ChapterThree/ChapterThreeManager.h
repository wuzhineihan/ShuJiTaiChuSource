// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Carriage.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "ChapterThreeManager.generated.h"

class AHorseEnemySpawnManager;
class UCarriageChaseSubsystem;
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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
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

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ChapterThree", meta=(AllowPrivateAccess=true, ClampMin="0.01"))
	float SpawnInterval = 5.0f;

	UPROPERTY()
	bool bStart = false;

private:
	void SyncBattleState();
	void HandleBattleStateChanged();
	void HandleBattleStopped(bool bReachedDestination);

	UFUNCTION()
	void HandleChapterStartBoxBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void EnsureChapterStartTrigger();
	void HandleChaseOver();

private:
	UPROPERTY(VisibleAnywhere, Category="ChapterThree", meta=(AllowPrivateAccess=true))
	TObjectPtr<USceneComponent> DefaultSceneRoot;

	UPROPERTY(VisibleAnywhere, Category="ChapterThree", meta=(AllowPrivateAccess=true))
	TObjectPtr<UBoxComponent> ChapterThreeStartBox;

	UPROPERTY(EditAnywhere, Category="ChapterThree|Trigger", meta=(ClampMin="0.0"))
	FVector ChapterStartBoxExtent = FVector(200.0f, 200.0f, 120.0f);

	UPROPERTY(EditAnywhere, Category="ChapterThree|Trigger")
	FName ChapterStartTriggerTag = TEXT("player");

	UPROPERTY(Transient)
	bool bChaseOverNotified = false;

	UPROPERTY(Transient)
	TObjectPtr<UCarriageChaseSubsystem> CachedChaseSubsystem;
};
