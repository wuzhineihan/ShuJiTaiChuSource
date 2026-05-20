// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/SacraEnemySubsystem.h"

#include "AI/SacraEnemyAIControllerBase.h"
#include "AI/Component/SacraEnemyHatredComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/ShapeComponent.h"
#include "Engine/World.h"
#include "Game/Characters/BaseEnemy.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

USacraEnemySubsystem* USacraEnemySubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	if (const UWorld* World = WorldContextObject->GetWorld())
	{
		return World->GetSubsystem<USacraEnemySubsystem>();
	}

	return nullptr;
}

void USacraEnemySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	RegisteredEnemies.Reset();
	HeavyPhaseCandidates.Reset();
	CachedPlayerActor.Reset();
}

void USacraEnemySubsystem::Deinitialize()
{
	StopPhaseTimers();
	RegisteredEnemies.Reset();
	HeavyPhaseCandidates.Reset();
	CachedPlayerActor.Reset();

	Super::Deinitialize();
}

void USacraEnemySubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	StartPhaseTimers();
}

void USacraEnemySubsystem::RegisterEnemy(ABaseEnemy* EnemyActor)
{
	if (!IsValid(EnemyActor))
	{
		return;
	}

	CompactRegisteredEnemies();

	for (const TWeakObjectPtr<ABaseEnemy>& RegisteredEnemy : RegisteredEnemies)
	{
		if (RegisteredEnemy.Get() == EnemyActor)
		{
			return;
		}
	}

	RegisteredEnemies.Add(EnemyActor);
}

void USacraEnemySubsystem::UnregisterEnemy(ABaseEnemy* EnemyActor)
{
	if (!EnemyActor)
	{
		return;
	}

	RegisteredEnemies.RemoveAll([EnemyActor](const TWeakObjectPtr<ABaseEnemy>& RegisteredEnemy)
	{
		return !RegisteredEnemy.IsValid() || RegisteredEnemy.Get() == EnemyActor;
	});

	HeavyPhaseCandidates.RemoveAll([EnemyActor](const TWeakObjectPtr<ABaseEnemy>& CandidateEnemy)
	{
		return !CandidateEnemy.IsValid() || CandidateEnemy.Get() == EnemyActor;
	});
}

bool USacraEnemySubsystem::ContainsEnemy(const ABaseEnemy* EnemyActor, bool bAliveOnly) const
{
	if (!EnemyActor)
	{
		return false;
	}

	CompactRegisteredEnemies();

	for (const TWeakObjectPtr<ABaseEnemy>& RegisteredEnemy : RegisteredEnemies)
	{
		const ABaseEnemy* CurrentEnemy = RegisteredEnemy.Get();
		if (CurrentEnemy != EnemyActor)
		{
			continue;
		}

		return !bAliveOnly || !CurrentEnemy->bIsDead;
	}

	return false;
}

void USacraEnemySubsystem::GetAllEnemies(TArray<ABaseEnemy*>& OutEnemies, bool bAliveOnly) const
{
	OutEnemies.Reset();
	CompactRegisteredEnemies();

	for (const TWeakObjectPtr<ABaseEnemy>& RegisteredEnemy : RegisteredEnemies)
	{
		ABaseEnemy* EnemyActor = RegisteredEnemy.Get();
		if (!PassesQueryFilter(EnemyActor, nullptr, bAliveOnly))
		{
			continue;
		}

		OutEnemies.Add(EnemyActor);
	}
}

int32 USacraEnemySubsystem::GetEnemyCount(bool bAliveOnly) const
{
	int32 Count = 0;
	CompactRegisteredEnemies();

	for (const TWeakObjectPtr<ABaseEnemy>& RegisteredEnemy : RegisteredEnemies)
	{
		if (PassesQueryFilter(RegisteredEnemy.Get(), nullptr, bAliveOnly))
		{
			++Count;
		}
	}

	return Count;
}

ABaseEnemy* USacraEnemySubsystem::FindEnemyByActor(const AActor* Actor, bool bAliveOnly) const
{
	CompactRegisteredEnemies();

	for (const TWeakObjectPtr<ABaseEnemy>& RegisteredEnemy : RegisteredEnemies)
	{
		ABaseEnemy* EnemyActor = RegisteredEnemy.Get();
		if (EnemyActor != Actor)
		{
			continue;
		}

		return PassesQueryFilter(EnemyActor, nullptr, bAliveOnly) ? EnemyActor : nullptr;
	}

	return nullptr;
}

ABaseEnemy* USacraEnemySubsystem::GetNearestEnemyToActor(const AActor* ReferenceActor, float MaxDistance, const AActor* IgnoreActor, bool bAliveOnly) const
{
	if (!IsValid(ReferenceActor))
	{
		return nullptr;
	}

	return GetNearestEnemyToLocation(ReferenceActor->GetActorLocation(), MaxDistance, IgnoreActor, bAliveOnly);
}

ABaseEnemy* USacraEnemySubsystem::GetNearestEnemyToLocation(const FVector& ReferenceLocation, float MaxDistance, const AActor* IgnoreActor, bool bAliveOnly) const
{
	CompactRegisteredEnemies();

	ABaseEnemy* BestEnemy = nullptr;
	float BestDistanceSquared = MaxDistance >= 0.0f ? FMath::Square(MaxDistance) : TNumericLimits<float>::Max();

	for (const TWeakObjectPtr<ABaseEnemy>& RegisteredEnemy : RegisteredEnemies)
	{
		ABaseEnemy* EnemyActor = RegisteredEnemy.Get();
		if (!PassesQueryFilter(EnemyActor, IgnoreActor, bAliveOnly))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(ReferenceLocation, EnemyActor->GetActorLocation());
		if (DistanceSquared > BestDistanceSquared)
		{
			continue;
		}

		BestDistanceSquared = DistanceSquared;
		BestEnemy = EnemyActor;
	}

	return BestEnemy;
}

void USacraEnemySubsystem::GetEnemiesInRange(const FVector& ReferenceLocation, float Radius, TArray<ABaseEnemy*>& OutEnemies, const AActor* IgnoreActor, bool bAliveOnly) const
{
	OutEnemies.Reset();
	CompactRegisteredEnemies();

	const float RadiusSquared = FMath::Square(FMath::Max(0.0f, Radius));
	for (const TWeakObjectPtr<ABaseEnemy>& RegisteredEnemy : RegisteredEnemies)
	{
		ABaseEnemy* EnemyActor = RegisteredEnemy.Get();
		if (!PassesQueryFilter(EnemyActor, IgnoreActor, bAliveOnly))
		{
			continue;
		}

		if (FVector::DistSquared(ReferenceLocation, EnemyActor->GetActorLocation()) <= RadiusSquared)
		{
			OutEnemies.Add(EnemyActor);
		}
	}
}

void USacraEnemySubsystem::RunHeavyPhase()
{
	CompactRegisteredEnemies();
	HeavyPhaseCandidates.Reset();

	AActor* PlayerActor = ResolvePlayerActor();
	if (!IsValid(PlayerActor))
	{
		for (const TWeakObjectPtr<ABaseEnemy>& RegisteredEnemy : RegisteredEnemies)
		{
			ApplyEnemyPhaseState(RegisteredEnemy.Get(), true, false);
		}
		return;
	}

	const FVector PlayerLocation = PlayerActor->GetActorLocation();
	const float HeavyRangeSquared = FMath::Square(FMath::Max(0.0f, HeavyRange));

	for (const TWeakObjectPtr<ABaseEnemy>& RegisteredEnemy : RegisteredEnemies)
	{
		ABaseEnemy* EnemyActor = RegisteredEnemy.Get();
		if (!PassesQueryFilter(EnemyActor, nullptr, true))
		{
			continue;
		}

		const bool bInHeavyRange = FVector::DistSquared(PlayerLocation, EnemyActor->GetActorLocation()) <= HeavyRangeSquared;
		if (bInHeavyRange)
		{
			HeavyPhaseCandidates.Add(EnemyActor);
			ApplyEnemyPhaseState(EnemyActor, false, true);
		}
		else
		{
			ApplyEnemyPhaseState(EnemyActor, true, false);
		}
	}
}

void USacraEnemySubsystem::RunLightPhase()
{
	CompactHeavyPhaseCandidates();

	AActor* PlayerActor = ResolvePlayerActor();
	if (!IsValid(PlayerActor))
	{
		for (const TWeakObjectPtr<ABaseEnemy>& CandidateEnemy : HeavyPhaseCandidates)
		{
			ApplyEnemyPhaseState(CandidateEnemy.Get(), true, false);
		}
		return;
	}

	const FVector PlayerLocation = PlayerActor->GetActorLocation();
	const float LightRangeSquared = FMath::Square(FMath::Max(0.0f, LightRange));

	for (const TWeakObjectPtr<ABaseEnemy>& CandidateEnemy : HeavyPhaseCandidates)
	{
		ABaseEnemy* EnemyActor = CandidateEnemy.Get();
		if (!PassesQueryFilter(EnemyActor, nullptr, true))
		{
			continue;
		}

		const bool bInLightRange = FVector::DistSquared(PlayerLocation, EnemyActor->GetActorLocation()) <= LightRangeSquared;
		if (bInLightRange)
		{
			ApplyEnemyPhaseState(EnemyActor, false, true);
		}
		else
		{
			ApplyEnemyPhaseState(EnemyActor, true, true);
		}
	}
}

void USacraEnemySubsystem::GetHeavyPhaseCandidates(TArray<ABaseEnemy*>& OutEnemies, bool bAliveOnly) const
{
	OutEnemies.Reset();
	CompactHeavyPhaseCandidates();

	for (const TWeakObjectPtr<ABaseEnemy>& CandidateEnemy : HeavyPhaseCandidates)
	{
		ABaseEnemy* EnemyActor = CandidateEnemy.Get();
		if (!PassesQueryFilter(EnemyActor, nullptr, bAliveOnly))
		{
			continue;
		}

		OutEnemies.Add(EnemyActor);
	}
}

int32 USacraEnemySubsystem::GetHeavyPhaseCandidateCount(bool bAliveOnly) const
{
	int32 Count = 0;
	CompactHeavyPhaseCandidates();

	for (const TWeakObjectPtr<ABaseEnemy>& CandidateEnemy : HeavyPhaseCandidates)
	{
		if (PassesQueryFilter(CandidateEnemy.Get(), nullptr, bAliveOnly))
		{
			++Count;
		}
	}

	return Count;
}

int32 USacraEnemySubsystem::BroadcastWarningAlert(ABaseEnemy* InstigatorEnemy, const FEnemyWarningAlertMessage& AlertMessage, float Radius, bool bAffectIdle, bool bAffectWarning, bool bAffectFight)
{
	if (!IsValid(InstigatorEnemy) || Radius < 0.0f)
	{
		return 0;
	}

	CompactRegisteredEnemies();

	const float RadiusSquared = FMath::Square(Radius);
	const FVector Origin = InstigatorEnemy->GetActorLocation();
	int32 AffectedCount = 0;

	for (const TWeakObjectPtr<ABaseEnemy>& RegisteredEnemy : RegisteredEnemies)
	{
		ABaseEnemy* EnemyActor = RegisteredEnemy.Get();
		if (!PassesQueryFilter(EnemyActor, InstigatorEnemy, true))
		{
			continue;
		}

		if (FVector::DistSquared(Origin, EnemyActor->GetActorLocation()) > RadiusSquared)
		{
			continue;
		}

		ASacraEnemyAIControllerBase* EnemyController = Cast<ASacraEnemyAIControllerBase>(EnemyActor->GetController());
		USacraEnemyHatredComponent* HatredComponent = EnemyController
			? EnemyController->FindComponentByClass<USacraEnemyHatredComponent>()
			: nullptr;
		if (!HatredComponent)
		{
			continue;
		}

		const EHatredState CurrentState = HatredComponent->GetCurrentHatredState();
		const bool bShouldAffect = (CurrentState == EHatredState::Idle && bAffectIdle)
			|| (CurrentState == EHatredState::Warning && bAffectWarning)
			|| (CurrentState == EHatredState::Fight && bAffectFight);
		if (!bShouldAffect)
		{
			continue;
		}

		if (HatredComponent->ApplyExternalWarningAlert(AlertMessage))
		{
			++AffectedCount;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy WarningAlert Broadcast Instigator=%s Radius=%.2f AlertLocation=%s FightTarget=%s Affected=%d"),
		*GetNameSafe(InstigatorEnemy),
		Radius,
		*AlertMessage.AlertLocation.ToCompactString(),
		*GetNameSafe(AlertMessage.FightTargetActor),
		AffectedCount);

	return AffectedCount;
}

void USacraEnemySubsystem::CompactRegisteredEnemies() const
{
	RegisteredEnemies.RemoveAll([](const TWeakObjectPtr<ABaseEnemy>& RegisteredEnemy)
	{
		return !RegisteredEnemy.IsValid();
	});
}

void USacraEnemySubsystem::CompactHeavyPhaseCandidates() const
{
	HeavyPhaseCandidates.RemoveAll([](const TWeakObjectPtr<ABaseEnemy>& CandidateEnemy)
	{
		return !CandidateEnemy.IsValid();
	});
}

bool USacraEnemySubsystem::PassesQueryFilter(const ABaseEnemy* EnemyActor, const AActor* IgnoreActor, bool bAliveOnly) const
{
	if (!IsValid(EnemyActor))
	{
		return false;
	}

	if (IgnoreActor && EnemyActor == IgnoreActor)
	{
		return false;
	}

	if (bAliveOnly && EnemyActor->bIsDead)
	{
		return false;
	}

	return true;
}

AActor* USacraEnemySubsystem::ResolvePlayerActor()
{
	if (CachedPlayerActor.IsValid())
	{
		return CachedPlayerActor.Get();
	}

	if (UWorld* World = GetWorld())
	{
		if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
		{
			CachedPlayerActor = PlayerPawn;
			return PlayerPawn;
		}
	}

	CachedPlayerActor.Reset();
	return nullptr;
}

void USacraEnemySubsystem::StartPhaseTimers()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FTimerManager& TimerManager = World->GetTimerManager();
	TimerManager.ClearTimer(HeavyPhaseTimerHandle);
	TimerManager.ClearTimer(LightPhaseTimerHandle);

	TimerManager.SetTimer(HeavyPhaseTimerHandle, this, &USacraEnemySubsystem::RunHeavyPhase, FMath::Max(0.05f, HeavyUpdateInterval), true);
	TimerManager.SetTimer(LightPhaseTimerHandle, this, &USacraEnemySubsystem::RunLightPhase, FMath::Max(0.01f, LightUpdateInterval), true);

	RunHeavyPhase();
	RunLightPhase();
}

void USacraEnemySubsystem::StopPhaseTimers()
{
	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(HeavyPhaseTimerHandle);
		TimerManager.ClearTimer(LightPhaseTimerHandle);
	}
}

void USacraEnemySubsystem::ApplyEnemyPhaseState(ABaseEnemy* EnemyActor, bool bPauseLogic, bool bEnableRendering) const
{
	if (!IsValid(EnemyActor))
	{
		return;
	}

	SetEnemyLogicPaused(EnemyActor, bPauseLogic);
	SetEnemyRenderingEnabled(EnemyActor, bEnableRendering);
}

void USacraEnemySubsystem::SetEnemyLogicPaused(ABaseEnemy* EnemyActor, bool bPaused) const
{
	if (!IsValid(EnemyActor))
	{
		return;
	}

	if (ASacraEnemyAIControllerBase* SacraController = Cast<ASacraEnemyAIControllerBase>(EnemyActor->GetController()))
	{
		SacraController->SetEnemyAIPaused(bPaused);
	}
}

void USacraEnemySubsystem::SetEnemyRenderingEnabled(ABaseEnemy* EnemyActor, bool bEnabled) const
{
	if (!IsValid(EnemyActor))
	{
		return;
	}

	if (ASacraEnemyAIControllerBase* SacraController = Cast<ASacraEnemyAIControllerBase>(EnemyActor->GetController()))
	{
		SacraController->SetEnemyRenderingEnabled(bEnabled);
	}

	EnemyActor->SetActorHiddenInGame(!bEnabled);

	TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents;
	EnemyActor->GetComponents(PrimitiveComponents);
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		// Shape components are collision-only helpers and should not have their
		// HiddenInGame state force-overridden by the enemy phase renderer.
		if (PrimitiveComponent->IsA<UShapeComponent>())
		{
			continue;
		}

		PrimitiveComponent->SetVisibility(bEnabled, true);
		PrimitiveComponent->SetHiddenInGame(!bEnabled, true);
	}
}
