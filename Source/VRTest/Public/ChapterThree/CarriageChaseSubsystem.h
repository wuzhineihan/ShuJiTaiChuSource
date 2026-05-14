#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CarriageChaseSubsystem.generated.h"

class ACarriage;
class ACartBase;
class AEnemyHorseBase;
class AHorseEnemySpawnManager;
class FSubsystemCollectionBase;
class USceneComponent;

DECLARE_MULTICAST_DELEGATE(FOnCarriageChaseStartedNative);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCarriageChaseStoppedNative, bool);
DECLARE_MULTICAST_DELEGATE(FOnCarriageChaseStateChangedNative);

USTRUCT(BlueprintType)
struct FCarriageChaseBattleConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarriageChase", meta=(ClampMin="0"))
	int32 MaxActiveEnemies = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarriageChase", meta=(ClampMin="0"))
	int32 InitialSpawnCount = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarriageChase", meta=(ClampMin="0.01"))
	float SpawnInterval = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarriageChase")
	bool bStartCarriageMovementOnBattleStart = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarriageChase")
	bool bMarkActiveEnemiesOverOnBattleEnd = true;
};

UCLASS()
class VRTEST_API UCarriageChaseSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UCarriageChaseSubsystem* Get(const UObject* WorldContextObject);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void RegisterCarriage(ACarriage* InCarriage);
	void UnregisterCarriage(ACarriage* InCarriage);

	void RegisterSpawnPoint(AHorseEnemySpawnManager* InSpawnPoint);
	void UnregisterSpawnPoint(AHorseEnemySpawnManager* InSpawnPoint);

	UFUNCTION(BlueprintCallable, Category="CarriageChase")
	void ConfigureBattle(const FCarriageChaseBattleConfig& InConfig);

	UFUNCTION(BlueprintCallable, Category="CarriageChase")
	bool StartBattle();

	UFUNCTION(BlueprintCallable, Category="CarriageChase")
	void StopBattle(bool bReachedDestination = false);

	UFUNCTION(BlueprintCallable, Category="CarriageChase")
	bool SpawnChaseEnemy();

	UFUNCTION(BlueprintPure, Category="CarriageChase")
	bool IsBattleActive() const { return bBattleActive; }

	UFUNCTION(BlueprintPure, Category="CarriageChase")
	ACarriage* GetCurrentCarriage() const;

	UFUNCTION(BlueprintPure, Category="CarriageChase")
	ACartBase* GetCurrentCart() const;

	UFUNCTION(BlueprintCallable, Category="CarriageChase")
	void GetActiveEnemies(TArray<AEnemyHorseBase*>& OutEnemies) const;

	UFUNCTION(BlueprintPure, Category="CarriageChase")
	int32 GetActiveEnemyCount() const;

	UFUNCTION(BlueprintCallable, Category="CarriageChase")
	USceneComponent* AssignChasePoint();

	UFUNCTION(BlueprintCallable, Category="CarriageChase")
	void ReleaseChasePoint(USceneComponent* ChasePoint);

	FOnCarriageChaseStartedNative OnBattleStarted;
	FOnCarriageChaseStoppedNative OnBattleStopped;
	FOnCarriageChaseStateChangedNative OnStateChanged;

private:
	void BroadcastStateChanged();
	void ClearSpawnTimer();
	void RemoveInvalidSpawnPoints();
	void RemoveInvalidActiveEnemies();
	void RegisterSpawnedEnemy(AEnemyHorseBase* SpawnedEnemy, USceneComponent* AssignedChasePoint);
	void UnregisterActiveEnemy(AActor* EnemyActor);
	AHorseEnemySpawnManager* SelectNearestSpawnPoint() const;
	void BindToCarriage(ACarriage* InCarriage);
	void UnbindFromCarriage(ACarriage* InCarriage);

	UFUNCTION()
	void HandleSpawnTimerFired();

	void HandleCarriageArrived(ACarriage* InCarriage);

	UFUNCTION()
	void HandleActiveEnemyDead(AActor* EnemyActor);

	UFUNCTION()
	void HandleActiveEnemyDestroyed(AActor* DestroyedActor);

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<ACarriage> CurrentCarriage;

	UPROPERTY(Transient)
	FCarriageChaseBattleConfig BattleConfig;

	UPROPERTY(Transient)
	bool bBattleActive = false;

	FTimerHandle SpawnEnemyTimerHandle;

	TArray<TWeakObjectPtr<AHorseEnemySpawnManager>> RegisteredSpawnPoints;
	TArray<TWeakObjectPtr<AEnemyHorseBase>> ActiveEnemies;
	TMap<TWeakObjectPtr<AActor>, TWeakObjectPtr<USceneComponent>> EnemyAssignedChasePoints;
};
