// Fill out your copyright notice in the Description page of Project Settings.


#include "ChapterThree/ChapterThreeManager.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "ChapterThree/CartBase.h"
#include "ChapterThree/EnemyHorseBase.h"
#include "ChapterThree/HorseEnemySpawnManager.h"

// Sets default values
AChapterThreeManager::AChapterThreeManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AChapterThreeManager::BeginPlay()
{
	Super::BeginPlay();
	for (TActorIterator<AHorseEnemySpawnManager> It(GetWorld()); It; ++It)
	{
		AHorseEnemySpawnManager* SpawnPoint = *It;
		if (SpawnPoint)
		{
			HorseEnemySpawnPoints.Add(SpawnPoint);
		}
	}	

	AActor* Carriage = UGameplayStatics::GetActorOfClass(GetWorld(),ACarriage::StaticClass());
	if (Carriage)
	{
		CurrentCarriage = Cast<ACarriage>(Carriage);
	}
}

// Called every frame
void AChapterThreeManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AChapterThreeManager::GenerateHorseEnemy()
{
	if (CurrentHorseEnemyNums >= MaxHorseEnemyNums)
	{
		return;
	}

	AHorseEnemySpawnManager* NearestHorseEnemySpawnPoint = nullptr;
	float MinDistance = FLT_MAX;
	
	for (auto i : HorseEnemySpawnPoints)
	{
		if (i)
		{
			float CurrentDistance = FVector::Dist(CurrentCarriage->GetCurrentCarriageLocation(),i->GetActorLocation());
			if (CurrentDistance < MinDistance)
			{
				MinDistance = CurrentDistance;
				NearestHorseEnemySpawnPoint  = i;
			}
		}
	}
	GEngine->AddOnScreenDebugMessage(-1,10,FColor::Red,"GenerateEnemy");
	if (NearestHorseEnemySpawnPoint)
	{
		AActor* NewEnemy = NearestHorseEnemySpawnPoint->GenerateEnemy(AssignChasePoint());
		if (NewEnemy)
		{
			AEnemyHorseBase* EnemyHorse = Cast<AEnemyHorseBase>(NewEnemy);
			if (EnemyHorse)
			{
				EnemyHorse->OnEnemyDead.AddDynamic(this,&AChapterThreeManager::OnEnemyDeadHandler);
			}
			CurrentEnemy.Add(NewEnemy);
		}
		
		
		
		CurrentHorseEnemyNums++;
	}
	
}

void AChapterThreeManager::CheckHorseEnemySpawnPoints(AHorseEnemySpawnManager* CurrentSpwanPoint)
{
	bool bCheck = HorseEnemySpawnPoints.Contains(CurrentSpwanPoint);
	if (!bCheck)
	{
		HorseEnemySpawnPoints.Add(CurrentSpwanPoint);
	}
}

void AChapterThreeManager::StartChapterThree()
{
	if (bStart)
	{
		return;
	}
	bStart = true;
	for (int i = 0;i<initHorseEnemyNums;i++)
	{
		GenerateHorseEnemy();
	}
	GetWorldTimerManager().SetTimer(GenerateEnemyTimerHandle,this,&AChapterThreeManager::GenerateHorseEnemy,5.0f,true);
	CurrentCarriage->SetMovable(true);
}

USceneComponent* AChapterThreeManager::AssignChasePoint()
{
	ACartBase* CurrentCart = Cast<ACartBase>(CurrentCarriage->CartActor);
	if (CurrentCart)
	{
		return CurrentCart->AssignChasePoint();
	}
	GEngine->AddOnScreenDebugMessage(-1,10,FColor::Red,"AssignChasePointFailed");
	return nullptr;
}

void AChapterThreeManager::OnEnemyDeadHandler(AActor* EnemyHorse)
{
	if (CurrentEnemy.Contains(EnemyHorse))
	{
		CurrentEnemy.Remove(EnemyHorse);
		CurrentHorseEnemyNums -= 1;
		if (CurrentHorseEnemyNums<0)
		{
			CurrentHorseEnemyNums = 0;
		}
		GEngine->AddOnScreenDebugMessage(-1,10,FColor::Red,FString::Printf(TEXT("Enemy Dead, Current Enemy Nums:%d"),CurrentHorseEnemyNums));
	}	
}