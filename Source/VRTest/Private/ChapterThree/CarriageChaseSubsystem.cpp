#include "ChapterThree/CarriageChaseSubsystem.h"

#include "ChapterThree/Carriage.h"
#include "ChapterThree/CartBase.h"
#include "ChapterThree/EnemyHorseBase.h"
#include "ChapterThree/HorseEnemySpawnManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UCarriageChaseSubsystem* UCarriageChaseSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	if (const UWorld* World = WorldContextObject->GetWorld())
	{
		return World->GetSubsystem<UCarriageChaseSubsystem>();
	}

	return nullptr;
}

void UCarriageChaseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RegisteredSpawnPoints.Reset();
	ActiveEnemies.Reset();
	EnemyAssignedChasePoints.Reset();
	CurrentCarriage.Reset();
	bBattleActive = false;
}

void UCarriageChaseSubsystem::Deinitialize()
{
	if (ACarriage* Carriage = CurrentCarriage.Get())
	{
		UnbindFromCarriage(Carriage);
	}

	ClearSpawnTimer();
	RegisteredSpawnPoints.Reset();
	ActiveEnemies.Reset();
	EnemyAssignedChasePoints.Reset();
	CurrentCarriage.Reset();
	bBattleActive = false;

	Super::Deinitialize();
}

void UCarriageChaseSubsystem::RegisterCarriage(ACarriage* InCarriage)
{
	if (!IsValid(InCarriage))
	{
		return;
	}

	if (CurrentCarriage.Get() == InCarriage)
	{
		return;
	}

	if (ACarriage* ExistingCarriage = CurrentCarriage.Get())
	{
		UnbindFromCarriage(ExistingCarriage);
	}

	CurrentCarriage = InCarriage;
	BindToCarriage(InCarriage);
	BroadcastStateChanged();
}

void UCarriageChaseSubsystem::UnregisterCarriage(ACarriage* InCarriage)
{
	if (CurrentCarriage.Get() != InCarriage)
	{
		return;
	}

	if (IsValid(InCarriage))
	{
		UnbindFromCarriage(InCarriage);
	}

	CurrentCarriage.Reset();
	if (bBattleActive)
	{
		StopBattle(false);
	}
	else
	{
		BroadcastStateChanged();
	}
}

void UCarriageChaseSubsystem::RegisterSpawnPoint(AHorseEnemySpawnManager* InSpawnPoint)
{
	if (!IsValid(InSpawnPoint))
	{
		return;
	}

	RemoveInvalidSpawnPoints();
	for (const TWeakObjectPtr<AHorseEnemySpawnManager>& RegisteredSpawnPoint : RegisteredSpawnPoints)
	{
		if (RegisteredSpawnPoint.Get() == InSpawnPoint)
		{
			return;
		}
	}

	RegisteredSpawnPoints.Add(InSpawnPoint);
}

void UCarriageChaseSubsystem::UnregisterSpawnPoint(AHorseEnemySpawnManager* InSpawnPoint)
{
	if (!InSpawnPoint)
	{
		return;
	}

	RegisteredSpawnPoints.RemoveAll([InSpawnPoint](const TWeakObjectPtr<AHorseEnemySpawnManager>& RegisteredSpawnPoint)
	{
		return !RegisteredSpawnPoint.IsValid() || RegisteredSpawnPoint.Get() == InSpawnPoint;
	});
}

void UCarriageChaseSubsystem::ConfigureBattle(const FCarriageChaseBattleConfig& InConfig)
{
	BattleConfig = InConfig;
}

bool UCarriageChaseSubsystem::StartBattle()
{
	if (bBattleActive)
	{
		return false;
	}

	ACarriage* Carriage = CurrentCarriage.Get();
	if (!IsValid(Carriage))
	{
		if (UWorld* World = GetWorld())
		{
			Carriage = Cast<ACarriage>(UGameplayStatics::GetActorOfClass(World, ACarriage::StaticClass()));
			if (IsValid(Carriage))
			{
				RegisterCarriage(Carriage);
			}
		}

		if (!IsValid(Carriage))
		{
			return false;
		}
	}

	bBattleActive = true;
	RemoveInvalidActiveEnemies();

	if (BattleConfig.bStartCarriageMovementOnBattleStart)
	{
		Carriage->SetMovable(true);
	}

	for (int32 SpawnIndex = 0; SpawnIndex < BattleConfig.InitialSpawnCount; ++SpawnIndex)
	{
		SpawnChaseEnemy();
	}

	ClearSpawnTimer();
	if (BattleConfig.SpawnInterval > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			SpawnEnemyTimerHandle,
			this,
			&UCarriageChaseSubsystem::HandleSpawnTimerFired,
			BattleConfig.SpawnInterval,
			true);
	}

	OnBattleStarted.Broadcast();
	BroadcastStateChanged();
	return true;
}

void UCarriageChaseSubsystem::StopBattle(bool bReachedDestination)
{
	if (!bBattleActive && !bReachedDestination)
	{
		return;
	}

	bBattleActive = false;
	ClearSpawnTimer();

	if (ACarriage* Carriage = CurrentCarriage.Get())
	{
		Carriage->SetMovable(false);
	}

	if (BattleConfig.bMarkActiveEnemiesOverOnBattleEnd)
	{
		TArray<AEnemyHorseBase*> EnemiesToMark;
		GetActiveEnemies(EnemiesToMark);
		for (AEnemyHorseBase* Enemy : EnemiesToMark)
		{
			if (IsValid(Enemy))
			{
				Enemy->SetOver(true);
			}
		}
	}

	for (const TPair<TWeakObjectPtr<AActor>, TWeakObjectPtr<USceneComponent>>& Pair : EnemyAssignedChasePoints)
	{
		if (Pair.Value.IsValid())
		{
			ReleaseChasePoint(Pair.Value.Get());
		}
	}
	EnemyAssignedChasePoints.Reset();

	BroadcastStateChanged();
	OnBattleStopped.Broadcast(bReachedDestination);
}

bool UCarriageChaseSubsystem::SpawnChaseEnemy()
{
	RemoveInvalidActiveEnemies();
	if (!bBattleActive)
	{
		return false;
	}

	if (GetActiveEnemyCount() >= BattleConfig.MaxActiveEnemies)
	{
		return false;
	}

	AHorseEnemySpawnManager* SpawnPoint = SelectNearestSpawnPoint();
	if (!IsValid(SpawnPoint))
	{
		return false;
	}

	USceneComponent* AssignedPoint = AssignChasePoint();
	if (!IsValid(AssignedPoint))
	{
		return false;
	}

	AEnemyHorseBase* SpawnedEnemy = Cast<AEnemyHorseBase>(SpawnPoint->GenerateEnemy(AssignedPoint));
	if (!IsValid(SpawnedEnemy))
	{
		ReleaseChasePoint(AssignedPoint);
		return false;
	}

	RegisterSpawnedEnemy(SpawnedEnemy, AssignedPoint);
	return true;
}

ACarriage* UCarriageChaseSubsystem::GetCurrentCarriage() const
{
	return CurrentCarriage.Get();
}

ACartBase* UCarriageChaseSubsystem::GetCurrentCart() const
{
	const ACarriage* Carriage = CurrentCarriage.Get();
	return IsValid(Carriage) ? Cast<ACartBase>(Carriage->CartActor) : nullptr;
}

void UCarriageChaseSubsystem::GetActiveEnemies(TArray<AEnemyHorseBase*>& OutEnemies) const
{
	OutEnemies.Reset();
	for (const TWeakObjectPtr<AEnemyHorseBase>& ActiveEnemy : ActiveEnemies)
	{
		if (AEnemyHorseBase* Enemy = ActiveEnemy.Get())
		{
			OutEnemies.Add(Enemy);
		}
	}
}

int32 UCarriageChaseSubsystem::GetActiveEnemyCount() const
{
	int32 Count = 0;
	for (const TWeakObjectPtr<AEnemyHorseBase>& ActiveEnemy : ActiveEnemies)
	{
		if (ActiveEnemy.IsValid())
		{
			++Count;
		}
	}

	return Count;
}

USceneComponent* UCarriageChaseSubsystem::AssignChasePoint()
{
	if (ACartBase* Cart = GetCurrentCart())
	{
		return Cart->AcquireChasePoint();
	}

	return nullptr;
}

void UCarriageChaseSubsystem::ReleaseChasePoint(USceneComponent* ChasePoint)
{
	if (ACartBase* Cart = GetCurrentCart())
	{
		Cart->ReleaseChasePoint(ChasePoint);
	}
}

void UCarriageChaseSubsystem::ClearSpawnTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SpawnEnemyTimerHandle);
	}
}

void UCarriageChaseSubsystem::BroadcastStateChanged()
{
	OnStateChanged.Broadcast();
}

void UCarriageChaseSubsystem::RemoveInvalidSpawnPoints()
{
	RegisteredSpawnPoints.RemoveAll([](const TWeakObjectPtr<AHorseEnemySpawnManager>& RegisteredSpawnPoint)
	{
		return !RegisteredSpawnPoint.IsValid();
	});
}

void UCarriageChaseSubsystem::RemoveInvalidActiveEnemies()
{
	const int32 PreviousAssignedPointCount = EnemyAssignedChasePoints.Num();
	const int32 PreviousActiveEnemyCount = ActiveEnemies.Num();
	TArray<TWeakObjectPtr<AActor>> KeysToRemove;
	for (const TPair<TWeakObjectPtr<AActor>, TWeakObjectPtr<USceneComponent>>& Pair : EnemyAssignedChasePoints)
	{
		if (!Pair.Key.IsValid())
		{
			if (Pair.Value.IsValid())
			{
				ReleaseChasePoint(Pair.Value.Get());
			}
			KeysToRemove.Add(Pair.Key);
		}
	}

	for (const TWeakObjectPtr<AActor>& Key : KeysToRemove)
	{
		EnemyAssignedChasePoints.Remove(Key);
	}

	ActiveEnemies.RemoveAll([](const TWeakObjectPtr<AEnemyHorseBase>& ActiveEnemy)
	{
		return !ActiveEnemy.IsValid();
	});

	if (EnemyAssignedChasePoints.Num() != PreviousAssignedPointCount || ActiveEnemies.Num() != PreviousActiveEnemyCount)
	{
		BroadcastStateChanged();
	}
}

void UCarriageChaseSubsystem::RegisterSpawnedEnemy(AEnemyHorseBase* SpawnedEnemy, USceneComponent* AssignedChasePoint)
{
	if (!IsValid(SpawnedEnemy))
	{
		return;
	}

	ActiveEnemies.Add(SpawnedEnemy);
	EnemyAssignedChasePoints.Add(TWeakObjectPtr<AActor>(SpawnedEnemy), AssignedChasePoint);
	SpawnedEnemy->OnEnemyDead.AddDynamic(this, &UCarriageChaseSubsystem::HandleActiveEnemyDead);
	SpawnedEnemy->OnDestroyed.AddDynamic(this, &UCarriageChaseSubsystem::HandleActiveEnemyDestroyed);
	BroadcastStateChanged();
}

void UCarriageChaseSubsystem::UnregisterActiveEnemy(AActor* EnemyActor)
{
	if (!EnemyActor)
	{
		return;
	}

	if (AEnemyHorseBase* EnemyHorse = Cast<AEnemyHorseBase>(EnemyActor))
	{
		EnemyHorse->OnEnemyDead.RemoveDynamic(this, &UCarriageChaseSubsystem::HandleActiveEnemyDead);
	}
	EnemyActor->OnDestroyed.RemoveDynamic(this, &UCarriageChaseSubsystem::HandleActiveEnemyDestroyed);

	const TWeakObjectPtr<AActor> EnemyKey(EnemyActor);
	if (const TWeakObjectPtr<USceneComponent>* AssignedPoint = EnemyAssignedChasePoints.Find(EnemyKey))
	{
		if (AssignedPoint->IsValid())
		{
			ReleaseChasePoint(AssignedPoint->Get());
		}
		EnemyAssignedChasePoints.Remove(EnemyKey);
	}

	ActiveEnemies.RemoveAll([EnemyActor](const TWeakObjectPtr<AEnemyHorseBase>& ActiveEnemy)
	{
		return !ActiveEnemy.IsValid() || ActiveEnemy.Get() == EnemyActor;
	});

	BroadcastStateChanged();
}

AHorseEnemySpawnManager* UCarriageChaseSubsystem::SelectNearestSpawnPoint() const
{
	const ACarriage* Carriage = CurrentCarriage.Get();
	if (!IsValid(Carriage))
	{
		return nullptr;
	}

	AHorseEnemySpawnManager* BestSpawnPoint = nullptr;
	float BestDistanceSquared = TNumericLimits<float>::Max();
	const FVector ReferenceLocation = Carriage->GetCurrentCarriageLocation();

	for (const TWeakObjectPtr<AHorseEnemySpawnManager>& RegisteredSpawnPoint : RegisteredSpawnPoints)
	{
		AHorseEnemySpawnManager* SpawnPoint = RegisteredSpawnPoint.Get();
		if (!IsValid(SpawnPoint))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(ReferenceLocation, SpawnPoint->GetActorLocation());
		if (DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestSpawnPoint = SpawnPoint;
		}
	}

	return BestSpawnPoint;
}

void UCarriageChaseSubsystem::BindToCarriage(ACarriage* InCarriage)
{
	if (IsValid(InCarriage))
	{
		InCarriage->OnCarriageArrived.AddUObject(this, &UCarriageChaseSubsystem::HandleCarriageArrived);
	}
}

void UCarriageChaseSubsystem::UnbindFromCarriage(ACarriage* InCarriage)
{
	if (IsValid(InCarriage))
	{
		InCarriage->OnCarriageArrived.RemoveAll(this);
	}
}

void UCarriageChaseSubsystem::HandleSpawnTimerFired()
{
	SpawnChaseEnemy();
}

void UCarriageChaseSubsystem::HandleCarriageArrived(ACarriage* InCarriage)
{
	if (bBattleActive && CurrentCarriage.Get() == InCarriage)
	{
		StopBattle(true);
	}
}

void UCarriageChaseSubsystem::HandleActiveEnemyDead(AActor* EnemyActor)
{
	UnregisterActiveEnemy(EnemyActor);
}

void UCarriageChaseSubsystem::HandleActiveEnemyDestroyed(AActor* DestroyedActor)
{
	UnregisterActiveEnemy(DestroyedActor);
}
