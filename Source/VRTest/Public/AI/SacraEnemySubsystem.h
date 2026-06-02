// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"

#include "SacraEnemySubsystem.generated.h"

class AActor;
class ABaseEnemy;
class FSubsystemCollectionBase;
struct FEnemyWarningAlertMessage;

UCLASS(Config = Game)
class VRTEST_API USacraEnemySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static USacraEnemySubsystem* Get(const UObject* WorldContextObject);
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	UFUNCTION(BlueprintCallable, Category = "AI|EnemySubsystem")
	void RegisterEnemy(ABaseEnemy* EnemyActor);

	UFUNCTION(BlueprintCallable, Category = "AI|EnemySubsystem")
	void UnregisterEnemy(ABaseEnemy* EnemyActor);

	UFUNCTION(BlueprintPure, Category = "AI|EnemySubsystem")
	bool ContainsEnemy(const ABaseEnemy* EnemyActor, bool bAliveOnly = false) const;

	UFUNCTION(BlueprintCallable, Category = "AI|EnemySubsystem")
	void GetAllEnemies(TArray<ABaseEnemy*>& OutEnemies, bool bAliveOnly = true) const;

	UFUNCTION(BlueprintPure, Category = "AI|EnemySubsystem")
	int32 GetEnemyCount(bool bAliveOnly = true) const;

	UFUNCTION(BlueprintPure, Category = "AI|EnemySubsystem")
	ABaseEnemy* FindEnemyByActor(const AActor* Actor, bool bAliveOnly = false) const;

	UFUNCTION(BlueprintPure, Category = "AI|EnemySubsystem")
	ABaseEnemy* GetNearestEnemyToActor(const AActor* ReferenceActor, float MaxDistance = -1.0f, const AActor* IgnoreActor = nullptr, bool bAliveOnly = true) const;

	UFUNCTION(BlueprintPure, Category = "AI|EnemySubsystem")
	ABaseEnemy* GetNearestEnemyToLocation(const FVector& ReferenceLocation, float MaxDistance = -1.0f, const AActor* IgnoreActor = nullptr, bool bAliveOnly = true) const;

	UFUNCTION(BlueprintCallable, Category = "AI|EnemySubsystem")
	void GetEnemiesInRange(const FVector& ReferenceLocation, float Radius, TArray<ABaseEnemy*>& OutEnemies, const AActor* IgnoreActor = nullptr, bool bAliveOnly = true) const;

	UFUNCTION(BlueprintCallable, Category = "AI|EnemySubsystem")
	void RunHeavyPhase();

	UFUNCTION(BlueprintCallable, Category = "AI|EnemySubsystem")
	void RunLightPhase();

	UFUNCTION(BlueprintCallable, Category = "AI|EnemySubsystem")
	void GetHeavyPhaseCandidates(TArray<ABaseEnemy*>& OutEnemies, bool bAliveOnly = true) const;

	UFUNCTION(BlueprintPure, Category = "AI|EnemySubsystem")
	int32 GetHeavyPhaseCandidateCount(bool bAliveOnly = true) const;

	UFUNCTION(BlueprintCallable, Category = "AI|EnemySubsystem")
	int32 BroadcastWarningAlert(ABaseEnemy* InstigatorEnemy, const FEnemyWarningAlertMessage& AlertMessage, float Radius, bool bAffectIdle = true, bool bAffectWarning = true, bool bAffectFight = false);

private:
	void CompactRegisteredEnemies() const;
	void CompactHeavyPhaseCandidates() const;
	bool PassesQueryFilter(const ABaseEnemy* EnemyActor, const AActor* IgnoreActor, bool bAliveOnly) const;
	AActor* ResolvePlayerActor();
	void StartPhaseTimers();
	void StopPhaseTimers();
	void ApplyEnemyPhaseState(ABaseEnemy* EnemyActor, bool bPauseLogic, bool bEnableRendering) const;
	void SetEnemyLogicPaused(ABaseEnemy* EnemyActor, bool bPaused) const;
	void SetEnemyRenderingEnabled(ABaseEnemy* EnemyActor, bool bEnabled) const;

private:
	UPROPERTY(EditAnywhere, Config, Category = "AI|EnemySubsystem|Phase", meta = (ClampMin = "0.05"))
	float HeavyUpdateInterval = 1.0f;

	UPROPERTY(EditAnywhere, Config, Category = "AI|EnemySubsystem|Phase", meta = (ClampMin = "0.01"))
	float LightUpdateInterval = 0.2f;

	UPROPERTY(EditAnywhere, Config, Category = "AI|EnemySubsystem|Phase", meta = (ClampMin = "0.0"))
	float HeavyRange = 5000.0f;

	UPROPERTY(EditAnywhere, Config, Category = "AI|EnemySubsystem|Phase", meta = (ClampMin = "0.0"))
	float LightRange = 3000.0f;

private:
	UPROPERTY(Transient)
	mutable TArray<TWeakObjectPtr<ABaseEnemy>> RegisteredEnemies;

	UPROPERTY(Transient)
	mutable TArray<TWeakObjectPtr<ABaseEnemy>> HeavyPhaseCandidates;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> CachedPlayerActor;

	FTimerHandle HeavyPhaseTimerHandle;
	FTimerHandle LightPhaseTimerHandle;
};
