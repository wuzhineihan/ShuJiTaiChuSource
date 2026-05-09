// Fill out your copyright notice in the Description page of Project Settings.


#include "ChapterThree/ChapterThreeManager.h"
#include "ChapterThree/CarriageChaseSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "ChapterThree/CartBase.h"
#include "ChapterThree/EnemyHorseBase.h"
#include "ChapterThree/HorseEnemySpawnManager.h"
#include "Components/BoxComponent.h"

// Sets default values
AChapterThreeManager::AChapterThreeManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	ChapterThreeStartBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ChapterThreeStartBOX_0"));
	ChapterThreeStartBox->SetupAttachment(DefaultSceneRoot);
	ChapterThreeStartBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ChapterThreeStartBox->SetCollisionObjectType(ECC_WorldDynamic);
	ChapterThreeStartBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	ChapterThreeStartBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ChapterThreeStartBox->SetGenerateOverlapEvents(true);
	ChapterThreeStartBox->SetBoxExtent(ChapterStartBoxExtent);

}

// Called when the game starts or when spawned
void AChapterThreeManager::BeginPlay()
{
	Super::BeginPlay();
	EnsureChapterStartTrigger();
	CachedChaseSubsystem = UCarriageChaseSubsystem::Get(this);
	if (CachedChaseSubsystem)
	{
		CachedChaseSubsystem->OnBattleStopped.AddUObject(this, &AChapterThreeManager::HandleBattleStopped);
		CurrentCarriage = CachedChaseSubsystem->GetCurrentCarriage();
	}

	SyncBattleState();
}

void AChapterThreeManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CachedChaseSubsystem)
	{
		CachedChaseSubsystem->OnBattleStopped.RemoveAll(this);
	}

	if (ChapterThreeStartBox)
	{
		ChapterThreeStartBox->OnComponentBeginOverlap.RemoveDynamic(this, &AChapterThreeManager::HandleChapterStartBoxBeginOverlap);
	}

	Super::EndPlay(EndPlayReason);
}

// Called every frame
void AChapterThreeManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SyncBattleState();
}

void AChapterThreeManager::GenerateHorseEnemy()
{
	if (CachedChaseSubsystem)
	{
		CachedChaseSubsystem->SpawnChaseEnemy();
	}
}

void AChapterThreeManager::CheckHorseEnemySpawnPoints(AHorseEnemySpawnManager* CurrentSpwanPoint)
{
	if (CachedChaseSubsystem)
	{
		CachedChaseSubsystem->RegisterSpawnPoint(CurrentSpwanPoint);
	}
}

void AChapterThreeManager::StartChapterThree()
{
	if (bStart)
	{
		return;
	}

	if (!CachedChaseSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("ChapterThreeManager Start failed: missing chase subsystem."));
		return;
	}

	if (!IsValid(CurrentCarriage))
	{
		CurrentCarriage = CachedChaseSubsystem->GetCurrentCarriage();
	}

	if (!IsValid(CurrentCarriage))
	{
		UE_LOG(LogTemp, Warning, TEXT("ChapterThreeManager Start failed: missing carriage."));
		return;
	}

	bStart = true;
	bChaseOverNotified = false;
	if (CachedChaseSubsystem)
	{
		FCarriageChaseBattleConfig BattleConfig;
		BattleConfig.MaxActiveEnemies = MaxHorseEnemyNums;
		BattleConfig.InitialSpawnCount = initHorseEnemyNums;
		BattleConfig.SpawnInterval = SpawnInterval;
		BattleConfig.bStartCarriageMovementOnBattleStart = true;
		BattleConfig.bMarkActiveEnemiesOverOnBattleEnd = true;
		CachedChaseSubsystem->ConfigureBattle(BattleConfig);
		CachedChaseSubsystem->StartBattle();
	}

	if (ChapterThreeStartBox)
	{
		ChapterThreeStartBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

USceneComponent* AChapterThreeManager::AssignChasePoint()
{
	if (CachedChaseSubsystem)
	{
		return CachedChaseSubsystem->AssignChasePoint();
	}

	return nullptr;
}

void AChapterThreeManager::OnEnemyDeadHandler(AActor* EnemyHorse)
{
	if (CachedChaseSubsystem)
	{
		SyncBattleState();
	}
}

void AChapterThreeManager::OnChaseOver_Implementation()
{
}

void AChapterThreeManager::HandleChapterStartBoxBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || bStart)
	{
		return;
	}

	if (ChapterStartTriggerTag.IsNone() || OtherActor->ActorHasTag(ChapterStartTriggerTag))
	{
		StartChapterThree();
	}
}

void AChapterThreeManager::EnsureChapterStartTrigger()
{
	if (!ChapterThreeStartBox)
	{
		return;
	}

	ChapterThreeStartBox->SetBoxExtent(ChapterStartBoxExtent);
	ChapterThreeStartBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ChapterThreeStartBox->OnComponentBeginOverlap.RemoveDynamic(this, &AChapterThreeManager::HandleChapterStartBoxBeginOverlap);
	ChapterThreeStartBox->OnComponentBeginOverlap.AddDynamic(this, &AChapterThreeManager::HandleChapterStartBoxBeginOverlap);
}

void AChapterThreeManager::HandleChaseOver()
{
	bChaseOverNotified = true;
	OnChaseOver();
}

void AChapterThreeManager::SyncBattleState()
{
	if (!CachedChaseSubsystem)
	{
		return;
	}

	CurrentCarriage = CachedChaseSubsystem->GetCurrentCarriage();
	CurrentHorseEnemyNums = CachedChaseSubsystem->GetActiveEnemyCount();
	CurrentEnemy.Reset();
	TArray<AEnemyHorseBase*> ActiveHorseEnemies;
	CachedChaseSubsystem->GetActiveEnemies(ActiveHorseEnemies);
	for (AEnemyHorseBase* ActiveHorseEnemy : ActiveHorseEnemies)
	{
		if (IsValid(ActiveHorseEnemy))
		{
			CurrentEnemy.Add(ActiveHorseEnemy);
		}
	}
}

void AChapterThreeManager::HandleBattleStopped(bool bReachedDestination)
{
	SyncBattleState();

	if (bReachedDestination && !bChaseOverNotified)
	{
		HandleChaseOver();
	}
}
