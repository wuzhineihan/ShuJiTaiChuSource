// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Component/SacraEnemyHatredComponent.h"

#include "AI/SacraEnemyAIControllerBase.h"
#include "AI/DataAsset/SacraEnemyHatredDataAsset.h"
#include "AISense_Player.h"
#include "Game/Characters/BaseEnemy.h"
#include "Game/MyGameplayTags.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AISense.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Sight.h"

namespace
{
const TCHAR* LexToString(const EHatredState InState)
{
	switch (InState)
	{
	case EHatredState::Idle:
		return TEXT("Idle");

	case EHatredState::Warning:
		return TEXT("Warning");

	case EHatredState::Fight:
		return TEXT("Fight");

	default:
		return TEXT("Unknown");
	}
}
}

USacraEnemyHatredComponent::USacraEnemyHatredComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USacraEnemyHatredComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USacraEnemyHatredComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindPerceptionDelegates();
	UnregisterGameplayMessageListener();
	bIsHatredInitialized = false;
	ClearSightLoseGraceTimers();

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(UpdateHatredValueTimerHandle);
	}

	ClearHatredDecayTimers();

	Super::EndPlay(EndPlayReason);
}

void USacraEnemyHatredComponent::InitHatredComponent()
{
	CachedEnemyAIController = Cast<ASacraEnemyAIControllerBase>(GetOwner());

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy Hatred Init Begin Owner=%s Controller=%s World=%s ConfigAsset=%s"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(CachedEnemyAIController.Get()),
		GetWorld() ? TEXT("Valid") : TEXT("Null"),
		*GetNameSafe(HatredConfigAsset));

	if (!IsValid(CachedEnemyAIController) || !GetWorld() || !IsValid(HatredConfigAsset))
	{
		UE_LOG(LogTemp, Warning, TEXT("SacraEnemy Hatred Init Failed Owner=%s Controller=%s World=%s ConfigAsset=%s"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(CachedEnemyAIController.Get()),
			GetWorld() ? TEXT("Valid") : TEXT("Null"),
			*GetNameSafe(HatredConfigAsset));
		bIsHatredInitialized = false;
		return;
	}

	bIsHatredInitialized = true;
	ResetHatredComponent();

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy Hatred Ready Owner=%s State=%s Value=%.2f WarningLocation=%s FightTarget=%s"),
		*GetNameSafe(GetOwner()),
		LexToString(CurrentHatredState),
		CurrentHatredValue,
		bHasWarningTargetLocation ? *CurrentWarningTargetLocation.ToCompactString() : TEXT("None"),
		*GetNameSafe(CurrentFightTargetActor.Get()));

	if (bIsHatredPaused)
	{
		return;
	}

	BindPerceptionDelegates();
	RegisterGameplayMessageListener();
	StartHatredValueUpdateTimer();
}

void USacraEnemyHatredComponent::ResetHatredComponent()
{
	ClearHatredDecayTimers();

	CachedSightHatredTargetMap.Reset();
	CachedHearingHatredTargetMap.Reset();
	CachedDamageHatredTargetMap.Reset();
	ClearSightLoseGraceTimers();

	RefreshPerceptionFlags();

	ClearWarningTargetLocation();
	ClearFightTargetActor();
	bHasLastKnownFightTargetLocation = false;
	LastKnownFightTargetLocation = FVector::ZeroVector;

	CurrentHatredValue = 0.0f;
	ChangeHatredState(EHatredState::Idle);
}

void USacraEnemyHatredComponent::SetHatredPaused(bool bInPaused)
{
	if (bIsHatredPaused == bInPaused)
	{
		return;
	}

	bIsHatredPaused = bInPaused;
	if (!bIsHatredInitialized)
	{
		return;
	}

	if (bIsHatredPaused)
	{
		UnbindPerceptionDelegates();
		UnregisterGameplayMessageListener();

		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(UpdateHatredValueTimerHandle);
		}

		ClearHatredDecayTimers();
		ClearSightLoseGraceTimers();
		return;
	}

	BindPerceptionDelegates();
	RegisterGameplayMessageListener();
	StartHatredValueUpdateTimer();

	if (!HasAnyPerception())
	{
		if (CurrentHatredState == EHatredState::Warning)
		{
			UpdateWarningStateDecayTimer();
		}
		else if (CurrentHatredState == EHatredState::Fight)
		{
			UpdateFightStateDecayTimer();
		}
	}
}

void USacraEnemyHatredComponent::OnPerceptionInfoUpdated(const FActorPerceptionUpdateInfo& UpdateInfo)
{
	if (bIsHatredPaused)
	{
		return;
	}

	AActor* TargetActor = UpdateInfo.Target.Get();
	if (!IsValid(TargetActor))
	{
		return;
	}

	const FAIStimulus& Stimulus = UpdateInfo.Stimulus;
	const FAISenseID SenseID = Stimulus.Type;
	const bool bWasSuccessfullySensed = Stimulus.WasSuccessfullySensed();
	const bool bOldSightPerceived = bIsSightPerceived;
	const bool bOldHearingPerceived = bIsHearingPerceived;
	const bool bOldDamagePerceived = bIsDamagePerceived;
	const bool bOldAnyPerception = HasAnyPerception();

	if (SenseID == UAISense::GetSenseID<UAISense_Player>())
	{
		if (bWasSuccessfullySensed)
		{
			CancelSightLoseGraceTimer(TargetActor);
			CachePerceptionStimulus(CachedSightHatredTargetMap, TargetActor, Stimulus);
			UpdateTargetLocation(Stimulus.StimulusLocation);
			UpdateTargetActor(TargetActor);
		}
		else
		{
			StartSightLoseGraceTimer(TargetActor);
		}
	}
	else if (SenseID == UAISense::GetSenseID<UAISense_Sight>())
	{
		if (bWasSuccessfullySensed)
		{
			TryApplyCorpseSightWarning(TargetActor, Stimulus);
		}
	}
	else if (SenseID == UAISense::GetSenseID<UAISense_Hearing>())
	{
		if (bWasSuccessfullySensed)
		{
			CachePerceptionStimulus(CachedHearingHatredTargetMap, TargetActor, Stimulus);
		}
		else
		{
			RemovePerceptionStimulus(CachedHearingHatredTargetMap, TargetActor);
		}
	}
	else if (SenseID == UAISense::GetSenseID<UAISense_Damage>())
	{
		if (bWasSuccessfullySensed)
		{
			CachePerceptionStimulus(CachedDamageHatredTargetMap, TargetActor, Stimulus);
			UpdateTargetLocation(Stimulus.StimulusLocation);
		}
		else
		{
			RemovePerceptionStimulus(CachedDamageHatredTargetMap, TargetActor);
		}
	}

	RefreshPerceptionFlags();

	const bool bNewAnyPerception = HasAnyPerception();
	if (bOldSightPerceived != bIsSightPerceived
		|| bOldHearingPerceived != bIsHearingPerceived
		|| bOldDamagePerceived != bIsDamagePerceived
		|| bOldAnyPerception != bNewAnyPerception)
	{
		UE_LOG(LogTemp, Log, TEXT("SacraEnemy Perception Changed Owner=%s Sight=%s->%s Hearing=%s->%s Damage=%s->%s Any=%s->%s"),
			*GetNameSafe(GetOwner()),
			bOldSightPerceived ? TEXT("true") : TEXT("false"),
			bIsSightPerceived ? TEXT("true") : TEXT("false"),
			bOldHearingPerceived ? TEXT("true") : TEXT("false"),
			bIsHearingPerceived ? TEXT("true") : TEXT("false"),
			bOldDamagePerceived ? TEXT("true") : TEXT("false"),
			bIsDamagePerceived ? TEXT("true") : TEXT("false"),
			bOldAnyPerception ? TEXT("true") : TEXT("false"),
			bNewAnyPerception ? TEXT("true") : TEXT("false"));
	}

	if (bNewAnyPerception)
	{
		ClearHatredDecayTimers();
	}
	else if (CurrentHatredState == EHatredState::Warning)
	{
		UpdateWarningStateDecayTimer();
	}
	else if (CurrentHatredState == EHatredState::Fight)
	{
		UpdateFightStateDecayTimer();
	}
}

void USacraEnemyHatredComponent::UpdateHatredValue()
{
	if (bIsHatredPaused)
	{
		return;
	}

	SyncSightStimuliFromPerception();

	float HatredDelta = 0.0f;

	switch (CurrentHatredState)
	{
	case EHatredState::Idle:
		ProcessIdleStateSense(HatredDelta);
		break;

	case EHatredState::Warning:
		ProcessWarningStateSense(HatredDelta);
		break;

	case EHatredState::Fight:
		ProcessFightStateSense(HatredDelta);
		break;

	default:
		break;
	}

	if (FMath::IsNearlyZero(HatredDelta))
	{
		HatredDelta = -GetConfiguredDefaultDecreaseHatredValueBase();
	}

	// UE_LOG(LogTemp, Log, TEXT("SacraEnemy Hatred Tick Owner=%s State=%s Delta=%.2f ValueBefore=%.2f Sight=%s Hearing=%s Damage=%s FightTarget=%s WarningLocation=%s"),
	// 	*GetNameSafe(GetOwner()),
	// 	LexToString(CurrentHatredState),
	// 	HatredDelta,
	// 	CurrentHatredValue,
	// 	bIsSightPerceived ? TEXT("true") : TEXT("false"),
	// 	bIsHearingPerceived ? TEXT("true") : TEXT("false"),
	// 	bIsDamagePerceived ? TEXT("true") : TEXT("false"),
	// 	*GetNameSafe(CurrentFightTargetActor.Get()),
	// 	bHasWarningTargetLocation ? *CurrentWarningTargetLocation.ToCompactString() : TEXT("None"));

	ApplyHatredDelta(HatredDelta);

}

bool USacraEnemyHatredComponent::ApplyExternalWarningAlert(const FEnemyWarningAlertMessage& AlertMessage)
{
	if (bIsHatredPaused || !bIsHatredInitialized)
	{
		return false;
	}

	if (!IsValid(GetOwner()) || AlertMessage.InstigatorActor == GetOwner())
	{
		return false;
	}

	const EHatredState PreviousState = CurrentHatredState;

	UpdateTargetLocation(AlertMessage.AlertLocation);
	if (IsValid(AlertMessage.FightTargetActor))
	{
		SetFightTargetActor(AlertMessage.FightTargetActor);
	}

	if (CurrentHatredState == EHatredState::Idle)
	{
		CurrentHatredValue = GetConfiguredWarningStateThreshold();
		ChangeHatredState(EHatredState::Warning);
		OnHatredValueChanged.Broadcast(CurrentHatredValue);
	}
	else if (CurrentHatredState == EHatredState::Warning)
	{
		OnHatredValueChanged.Broadcast(CurrentHatredValue);
	}
	else
	{
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy WarningAlert Applied Owner=%s Source=%s OldState=%s NewState=%s AlertLocation=%s FightTarget=%s"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(AlertMessage.InstigatorActor),
		LexToString(PreviousState),
		LexToString(CurrentHatredState),
		*AlertMessage.AlertLocation.ToCompactString(),
		*GetNameSafe(AlertMessage.FightTargetActor));

	return true;
}

float USacraEnemyHatredComponent::GetMaxHatredValue() const
{
	return GetConfiguredMaxHatredValue();
}

float USacraEnemyHatredComponent::GetConfiguredUpdateHatredValueInterval() const
{
	if (!IsValid(HatredConfigAsset))
	{
		return 0.2f;
	}

	return HatredConfigAsset->UpdateHatredValueInterval;
}

float USacraEnemyHatredComponent::GetConfiguredWarningStateDecayInterval() const
{
	if (!IsValid(HatredConfigAsset))
	{
		return 10.0f;
	}

	return HatredConfigAsset->WarningStateDecayInterval;
}

float USacraEnemyHatredComponent::GetConfiguredFightStateDecayInterval() const
{
	if (!IsValid(HatredConfigAsset))
	{
		return 10.0f;
	}

	return HatredConfigAsset->FightStateDecayInterval;
}

float USacraEnemyHatredComponent::GetConfiguredSightLoseGraceInterval() const
{
	if (!IsValid(HatredConfigAsset))
	{
		return 0.35f;
	}

	return HatredConfigAsset->SightLoseGraceInterval;
}

float USacraEnemyHatredComponent::GetConfiguredIdleSightGrowthBase() const
{
	if (!IsValid(HatredConfigAsset))
	{
		return 1.0f;
	}

	return HatredConfigAsset->IdleSightGrowthBase;
}

float USacraEnemyHatredComponent::GetConfiguredWarningSightGrowthBase() const
{
	if (!IsValid(HatredConfigAsset))
	{
		return 2.0f;
	}

	return HatredConfigAsset->WarningSightGrowthBase;
}

float USacraEnemyHatredComponent::GetConfiguredDefaultDecreaseHatredValueBase() const
{
	if (!IsValid(HatredConfigAsset))
	{
		return 1.0f;
	}

	return HatredConfigAsset->DefaultDecreaseHatredValueBase;
}

float USacraEnemyHatredComponent::GetConfiguredMaxHatredValue() const
{
	if (!IsValid(HatredConfigAsset))
	{
		return 100.0f;
	}

	return HatredConfigAsset->MaxHatredValue;
}

float USacraEnemyHatredComponent::GetConfiguredWarningStateThreshold() const
{
	if (!IsValid(HatredConfigAsset))
	{
		return 50.0f;
	}

	return HatredConfigAsset->WarningStateThreshold;
}

void USacraEnemyHatredComponent::OnWarningStateExitMessage(FGameplayTag Channel, const FEnemyHatredStateMessage& Message)
{
	if (bIsHatredPaused)
	{
		return;
	}

	if (Channel != MyProjectTags::TAG_AI_Message_Hatred_WarningExitToIdle)
	{
		return;
	}

	if (CurrentHatredState != EHatredState::Warning || HasAnyPerception())
	{
		return;
	}

	if (Message.InstigatorActor == nullptr || Message.InstigatorActor != GetOwner())
	{
		return;
	}

	ClearWarningTargetLocation();
	CurrentHatredValue = 0.0f;
	ChangeHatredState(EHatredState::Idle);
	OnHatredValueChanged.Broadcast(CurrentHatredValue);
}

void USacraEnemyHatredComponent::BindPerceptionDelegates()
{
	UnbindPerceptionDelegates();

	if (!IsValid(CachedEnemyAIController))
	{
		UE_LOG(LogTemp, Warning, TEXT("SacraEnemy Hatred BindPerception Failed Owner=%s Reason=InvalidController"),
			*GetNameSafe(GetOwner()));
		return;
	}

	if (UAIPerceptionComponent* PerceptionComponent = ASacraEnemyAIControllerBase::FindPerceptionComponent(CachedEnemyAIController))
	{
		PerceptionComponent->OnTargetPerceptionInfoUpdated.AddDynamic(this, &USacraEnemyHatredComponent::OnPerceptionInfoUpdated);
		UE_LOG(LogTemp, Log, TEXT("SacraEnemy Hatred BindPerception Success Owner=%s Perception=%s"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(PerceptionComponent));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SacraEnemy Hatred BindPerception Failed Owner=%s Reason=MissingPerceptionComponent"),
			*GetNameSafe(GetOwner()));
	}
}

void USacraEnemyHatredComponent::UnbindPerceptionDelegates()
{
	if (!IsValid(CachedEnemyAIController))
	{
		return;
	}

	if (UAIPerceptionComponent* PerceptionComponent = ASacraEnemyAIControllerBase::FindPerceptionComponent(CachedEnemyAIController))
	{
		PerceptionComponent->OnTargetPerceptionInfoUpdated.RemoveDynamic(this, &USacraEnemyHatredComponent::OnPerceptionInfoUpdated);
	}
}

void USacraEnemyHatredComponent::RegisterGameplayMessageListener()
{
	UnregisterGameplayMessageListener();

	if (!UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	WarningStateExitMessageHandle = MessageSubsystem.RegisterListener<FEnemyHatredStateMessage>(
		MyProjectTags::TAG_AI_Message_Hatred_WarningExitToIdle,
		this,
		&USacraEnemyHatredComponent::OnWarningStateExitMessage);
}

void USacraEnemyHatredComponent::UnregisterGameplayMessageListener()
{
	if (WarningStateExitMessageHandle.IsValid())
	{
		WarningStateExitMessageHandle.Unregister();
	}
}

void USacraEnemyHatredComponent::ChangeHatredState(EHatredState NewState)
{
	if (CurrentHatredState == NewState)
	{
		return;
	}

	const EHatredState OldState = CurrentHatredState;
	ClearHatredDecayTimers();

	CurrentHatredState = NewState;

	if (CurrentHatredState == EHatredState::Idle)
	{
		ClearFightTargetActor();
	}
	else if (CurrentHatredState == EHatredState::Fight)
	{
		UpdateFightTargetFromSight();
	}

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy State Changed Owner=%s Old=%s New=%s Value=%.2f WarningLocation=%s FightTarget=%s"),
		*GetNameSafe(GetOwner()),
		LexToString(OldState),
		LexToString(CurrentHatredState),
		CurrentHatredValue,
		bHasWarningTargetLocation ? *CurrentWarningTargetLocation.ToCompactString() : TEXT("None"),
		*GetNameSafe(CurrentFightTargetActor.Get()));

	OnHatredStateChanged.Broadcast(CurrentHatredState);
}

void USacraEnemyHatredComponent::ApplyHatredDelta(float DeltaValue)
{
	switch (CurrentHatredState)
	{
	case EHatredState::Idle:
		CurrentHatredValue = FMath::Clamp(CurrentHatredValue + DeltaValue, 0.0f, GetConfiguredWarningStateThreshold());
		if (CurrentHatredValue >= GetConfiguredWarningStateThreshold())
		{
			UE_LOG(LogTemp, Log, TEXT("SacraEnemy Hatred ThresholdReached Owner=%s From=Idle To=Warning Delta=%.2f Value=%.2f Threshold=%.2f"),
				*GetNameSafe(GetOwner()),
				DeltaValue,
				CurrentHatredValue,
				GetConfiguredWarningStateThreshold());
			CurrentHatredValue = GetConfiguredWarningStateThreshold();
			ChangeHatredState(EHatredState::Warning);
		}
		break;

	case EHatredState::Warning:
		CurrentHatredValue = FMath::Clamp(CurrentHatredValue + DeltaValue, GetConfiguredWarningStateThreshold(), GetConfiguredMaxHatredValue());
		if (CurrentHatredValue >= GetConfiguredMaxHatredValue())
		{
			UE_LOG(LogTemp, Log, TEXT("SacraEnemy Hatred ThresholdReached Owner=%s From=Warning To=Fight Delta=%.2f Value=%.2f Max=%.2f"),
				*GetNameSafe(GetOwner()),
				DeltaValue,
				CurrentHatredValue,
				GetConfiguredMaxHatredValue());
			CurrentHatredValue = GetConfiguredMaxHatredValue();
			UpdateFightTargetFromSight();
			ChangeHatredState(EHatredState::Fight);
		}
		break;

	case EHatredState::Fight:
		CurrentHatredValue = GetConfiguredMaxHatredValue();
		break;

	default:
		break;
	}

	OnHatredValueChanged.Broadcast(CurrentHatredValue);
}

void USacraEnemyHatredComponent::ProcessIdleStateSense(float& OutDeltaValue)
{
	for (const TPair<TWeakObjectPtr<AActor>, FAIStimulus>& Pair : CachedSightHatredTargetMap)
	{
		if (!IsValid(Pair.Key.Get()))
		{
			continue;
		}

		OutDeltaValue += GetConfiguredIdleSightGrowthBase() * Pair.Value.Strength;
	}

	FVector NearestHearingLocation = FVector::ZeroVector;
	if (TryGetNearestStimulusLocation(CachedHearingHatredTargetMap, NearestHearingLocation))
	{
		UpdateTargetLocation(NearestHearingLocation);
		OutDeltaValue = FMath::Max(OutDeltaValue, GetConfiguredWarningStateThreshold());
	}

	FVector NearestDamageLocation = FVector::ZeroVector;
	if (TryGetNearestStimulusLocation(CachedDamageHatredTargetMap, NearestDamageLocation))
	{
		UpdateTargetLocation(NearestDamageLocation);
		OutDeltaValue = FMath::Max(OutDeltaValue, GetConfiguredWarningStateThreshold());
	}

	ConsumeTransientPerceptionStimuli();
}

void USacraEnemyHatredComponent::ProcessWarningStateSense(float& OutDeltaValue)
{
	for (const TPair<TWeakObjectPtr<AActor>, FAIStimulus>& Pair : CachedSightHatredTargetMap)
	{
		AActor* TargetActor = Pair.Key.Get();
		if (!IsValid(TargetActor))
		{
			continue;
		}

		UpdateTargetLocation(Pair.Value.StimulusLocation);
		UpdateTargetActor(TargetActor);
		OutDeltaValue += GetConfiguredWarningSightGrowthBase() * Pair.Value.Strength;
	}

	UpdateFightTargetFromSight();
	if (IsValid(CurrentFightTargetActor))
	{
		UpdateTargetLocation(CurrentFightTargetActor->GetActorLocation());
	}

	FVector NearestHearingLocation = FVector::ZeroVector;
	if (TryGetNearestStimulusLocation(CachedHearingHatredTargetMap, NearestHearingLocation))
	{
		UpdateTargetLocation(NearestHearingLocation);
	}

	FVector NearestDamageLocation = FVector::ZeroVector;
	if (TryGetNearestStimulusLocation(CachedDamageHatredTargetMap, NearestDamageLocation))
	{
		UpdateTargetLocation(NearestDamageLocation);
	}

	if (HasAnyPerception())
	{
		ClearHatredDecayTimers();
	}
	else
	{
		UpdateWarningStateDecayTimer();
	}

	ConsumeTransientPerceptionStimuli();
}

void USacraEnemyHatredComponent::ProcessFightStateSense(float& OutDeltaValue)
{
	OutDeltaValue = 0.0f;

	UpdateFightTargetFromSight();

	if (HasAnyPerception())
	{
		ClearHatredDecayTimers();
	}
	else
	{
		UpdateFightStateDecayTimer();
	}

	ConsumeTransientPerceptionStimuli();
}

void USacraEnemyHatredComponent::RefreshPerceptionFlags()
{
	CleanupStimulusMap(CachedSightHatredTargetMap);
	CleanupStimulusMap(CachedHearingHatredTargetMap);
	CleanupStimulusMap(CachedDamageHatredTargetMap);

	bIsSightPerceived = CachedSightHatredTargetMap.Num() > 0;
	bIsHearingPerceived = CachedHearingHatredTargetMap.Num() > 0;
	bIsDamagePerceived = CachedDamageHatredTargetMap.Num() > 0;
}

bool USacraEnemyHatredComponent::HasAnyPerception() const
{
	return bIsSightPerceived || bIsHearingPerceived || bIsDamagePerceived;
}

void USacraEnemyHatredComponent::SyncSightStimuliFromPerception()
{
	if (!IsValid(CachedEnemyAIController))
	{
		return;
	}

	UAIPerceptionComponent* PerceptionComponent = ASacraEnemyAIControllerBase::FindPerceptionComponent(CachedEnemyAIController);
	if (!IsValid(PerceptionComponent))
	{
		return;
	}

	TArray<AActor*> CurrentlyPerceivedActors;
	PerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Player::StaticClass(), CurrentlyPerceivedActors);

	TSet<TWeakObjectPtr<AActor>> ValidSightActors;
	for (AActor* PerceivedActor : CurrentlyPerceivedActors)
	{
		if (!IsValid(PerceivedActor))
		{
			continue;
		}

		FAIStimulus SightStimulus;
		if (HasSuccessfulSightStimulusFromPerception(PerceivedActor, &SightStimulus))
		{
			CancelSightLoseGraceTimer(PerceivedActor);
			CachedSightHatredTargetMap.Add(PerceivedActor, SightStimulus);
			ValidSightActors.Add(PerceivedActor);
		}
	}

	for (auto It = CachedSightHatredTargetMap.CreateIterator(); It; ++It)
	{
		FAIStimulus SightStimulus;
		if (HasSuccessfulSightStimulusFromPerception(It->Key.Get(), &SightStimulus))
		{
			CancelSightLoseGraceTimer(It->Key.Get());
			It->Value = SightStimulus;
			ValidSightActors.Add(It->Key);
			continue;
		}

		if (!ValidSightActors.Contains(It->Key))
		{
			StartSightLoseGraceTimer(It->Key.Get());
		}
	}

	RefreshPerceptionFlags();
}

bool USacraEnemyHatredComponent::HasSuccessfulSightStimulusFromPerception(AActor* TargetActor, FAIStimulus* OutStimulus) const
{
	if (!IsValid(CachedEnemyAIController) || !IsValid(TargetActor))
	{
		return false;
	}

	UAIPerceptionComponent* PerceptionComponent = ASacraEnemyAIControllerBase::FindPerceptionComponent(CachedEnemyAIController);
	if (!IsValid(PerceptionComponent))
	{
		return false;
	}

	FActorPerceptionBlueprintInfo PerceptionInfo;
	PerceptionComponent->GetActorsPerception(TargetActor, PerceptionInfo);

	for (const FAIStimulus& Stimulus : PerceptionInfo.LastSensedStimuli)
	{
		if (Stimulus.Type == UAISense::GetSenseID<UAISense_Player>() && Stimulus.WasSuccessfullySensed())
		{
			if (OutStimulus)
			{
				*OutStimulus = Stimulus;
			}

			return true;
		}
	}

	return false;
}

void USacraEnemyHatredComponent::CachePerceptionStimulus(TMap<TWeakObjectPtr<AActor>, FAIStimulus>& StimulusMap, AActor* TargetActor, const FAIStimulus& Stimulus)
{
	if (IsValid(TargetActor))
	{
		StimulusMap.Add(TargetActor, Stimulus);
	}
}

void USacraEnemyHatredComponent::RemovePerceptionStimulus(TMap<TWeakObjectPtr<AActor>, FAIStimulus>& StimulusMap, AActor* TargetActor)
{
	if (IsValid(TargetActor))
	{
		StimulusMap.Remove(TargetActor);
	}
}

void USacraEnemyHatredComponent::CleanupStimulusMap(TMap<TWeakObjectPtr<AActor>, FAIStimulus>& StimulusMap) const
{
	for (auto It = StimulusMap.CreateIterator(); It; ++It)
	{
		if (!IsValid(It->Key.Get()))
		{
			It.RemoveCurrent();
		}
	}
}

void USacraEnemyHatredComponent::ConsumeTransientPerceptionStimuli()
{
	CachedHearingHatredTargetMap.Reset();
	CachedDamageHatredTargetMap.Reset();
	RefreshPerceptionFlags();
}

void USacraEnemyHatredComponent::StartSightLoseGraceTimer(AActor* TargetActor)
{
	if (!IsValid(TargetActor) || !GetWorld())
	{
		return;
	}

	const float GraceInterval = GetConfiguredSightLoseGraceInterval();
	if (GraceInterval <= 0.0f)
	{
		HandleSightLoseGraceTimerExpired(TargetActor);
		return;
	}

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	FTimerHandle& TimerHandle = PendingSightLoseTimerHandleMap.FindOrAdd(TargetActor);
	if (TimerManager.IsTimerActive(TimerHandle))
	{
		return;
	}

	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &USacraEnemyHatredComponent::HandleSightLoseGraceTimerExpired, TWeakObjectPtr<AActor>(TargetActor));
	TimerManager.SetTimer(TimerHandle, TimerDelegate, GraceInterval, false);
}

void USacraEnemyHatredComponent::CancelSightLoseGraceTimer(AActor* TargetActor)
{
	if (!IsValid(TargetActor) || !GetWorld())
	{
		return;
	}

	FTimerHandle* TimerHandle = PendingSightLoseTimerHandleMap.Find(TargetActor);
	if (!TimerHandle)
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(*TimerHandle);
	PendingSightLoseTimerHandleMap.Remove(TargetActor);
}

void USacraEnemyHatredComponent::ClearSightLoseGraceTimers()
{
	if (GetWorld())
	{
		FTimerManager& TimerManager = GetWorld()->GetTimerManager();
		for (TPair<TWeakObjectPtr<AActor>, FTimerHandle>& Pair : PendingSightLoseTimerHandleMap)
		{
			TimerManager.ClearTimer(Pair.Value);
		}
	}

	PendingSightLoseTimerHandleMap.Reset();
}

void USacraEnemyHatredComponent::HandleSightLoseGraceTimerExpired(TWeakObjectPtr<AActor> TargetActor)
{
	if (!GetWorld())
	{
		PendingSightLoseTimerHandleMap.Remove(TargetActor);
		return;
	}

	FTimerHandle* TimerHandle = PendingSightLoseTimerHandleMap.Find(TargetActor);
	if (TimerHandle)
	{
		GetWorld()->GetTimerManager().ClearTimer(*TimerHandle);
		PendingSightLoseTimerHandleMap.Remove(TargetActor);
	}

	AActor* ResolvedTargetActor = TargetActor.Get();
	FAIStimulus RefreshedStimulus;
	if (HasSuccessfulSightStimulusFromPerception(ResolvedTargetActor, &RefreshedStimulus))
	{
		CachePerceptionStimulus(CachedSightHatredTargetMap, ResolvedTargetActor, RefreshedStimulus);
		RefreshPerceptionFlags();
		return;
	}

	if (CurrentFightTargetActor == ResolvedTargetActor)
	{
		CacheLastKnownLocationFromFightTarget();
		UE_LOG(LogTemp, Log, TEXT("SacraEnemy FightTarget LostSight Owner=%s Target=%s CachedWarningLocation=%s Grace=%.2f"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(ResolvedTargetActor),
			bHasWarningTargetLocation ? *CurrentWarningTargetLocation.ToCompactString() : TEXT("None"),
			GetConfiguredSightLoseGraceInterval());
	}

	RemovePerceptionStimulus(CachedSightHatredTargetMap, ResolvedTargetActor);
	RefreshPerceptionFlags();
	UpdateFightTargetFromSight();

	if (HasAnyPerception())
	{
		ClearHatredDecayTimers();
	}
	else if (CurrentHatredState == EHatredState::Warning)
	{
		UpdateWarningStateDecayTimer();
	}
	else if (CurrentHatredState == EHatredState::Fight)
	{
		UpdateFightStateDecayTimer();
	}
}

void USacraEnemyHatredComponent::UpdateTargetLocation(const FVector& InLocation)
{
	if (!IsValid(GetOwner()))
	{
		return;
	}

	CurrentWarningTargetLocation = InLocation;
	bHasWarningTargetLocation = true;
	RememberLastKnownFightTargetLocation(InLocation);
}

void USacraEnemyHatredComponent::UpdateTargetActor(AActor* InActor)
{
	if (IsValid(InActor))
	{
		SetFightTargetActor(InActor);
	}
}

bool USacraEnemyHatredComponent::TryApplyCorpseSightWarning(AActor* TargetActor, const FAIStimulus& Stimulus)
{
	ABaseEnemy* SensedEnemy = Cast<ABaseEnemy>(TargetActor);
	if (!IsValid(SensedEnemy) || SensedEnemy == GetOwner() || !SensedEnemy->bIsDead)
	{
		return false;
	}

	UpdateTargetLocation(Stimulus.StimulusLocation);
	if (CurrentHatredState == EHatredState::Idle)
	{
		CurrentHatredValue = GetConfiguredWarningStateThreshold();
		ChangeHatredState(EHatredState::Warning);
		OnHatredValueChanged.Broadcast(CurrentHatredValue);
	}
	else if (CurrentHatredState == EHatredState::Warning)
	{
		CurrentHatredValue = FMath::Max(CurrentHatredValue, GetConfiguredWarningStateThreshold());
		OnHatredValueChanged.Broadcast(CurrentHatredValue);
	}
	else
	{
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy CorpseSight Warning Owner=%s Corpse=%s Location=%s State=%s"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(TargetActor),
		*Stimulus.StimulusLocation.ToCompactString(),
		LexToString(CurrentHatredState));

	return true;
}

void USacraEnemyHatredComponent::SetFightTargetActor(AActor* InActor)
{
	if (CurrentFightTargetActor.Get() == InActor)
	{
		return;
	}

	CurrentFightTargetActor = InActor;
	OnFightTargetChanged.Broadcast(InActor);
}

void USacraEnemyHatredComponent::UpdateFightTargetFromSight()
{
	AActor* PreviousFightTargetActor = CurrentFightTargetActor.Get();
	AActor* BestTargetActor = nullptr;
	float BestDistanceSquared = TNumericLimits<float>::Max();
	const FVector OwnerLocation = IsValid(GetOwner()) ? GetOwner()->GetActorLocation() : FVector::ZeroVector;

	for (const TPair<TWeakObjectPtr<AActor>, FAIStimulus>& Pair : CachedSightHatredTargetMap)
	{
		AActor* CandidateActor = Pair.Key.Get();
		if (!IsValid(CandidateActor))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(OwnerLocation, CandidateActor->GetActorLocation());
		if (DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestTargetActor = CandidateActor;
		}
	}

	if (IsValid(BestTargetActor))
	{
		SetFightTargetActor(BestTargetActor);
		const FVector BestTargetLocation = BestTargetActor->GetActorLocation();
		CurrentWarningTargetLocation = BestTargetLocation;
		bHasWarningTargetLocation = true;
		RememberLastKnownFightTargetLocation(BestTargetLocation);
	}

	if (PreviousFightTargetActor != CurrentFightTargetActor.Get())
	{
		UE_LOG(LogTemp, Log, TEXT("SacraEnemy FightTarget Updated Owner=%s Old=%s New=%s WarningLocation=%s"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(PreviousFightTargetActor),
			*GetNameSafe(CurrentFightTargetActor.Get()),
			bHasWarningTargetLocation ? *CurrentWarningTargetLocation.ToCompactString() : TEXT("None"));
	}
}

bool USacraEnemyHatredComponent::TryGetNearestStimulusLocation(const TMap<TWeakObjectPtr<AActor>, FAIStimulus>& StimulusMap, FVector& OutLocation) const
{
	if (!IsValid(GetOwner()))
	{
		return false;
	}

	bool bFoundLocation = false;
	float BestDistanceSquared = TNumericLimits<float>::Max();
	const FVector OwnerLocation = GetOwner()->GetActorLocation();

	for (const TPair<TWeakObjectPtr<AActor>, FAIStimulus>& Pair : StimulusMap)
	{
		if (!IsValid(Pair.Key.Get()))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(OwnerLocation, Pair.Value.StimulusLocation);
		if (!bFoundLocation || DistanceSquared < BestDistanceSquared)
		{
			bFoundLocation = true;
			BestDistanceSquared = DistanceSquared;
			OutLocation = Pair.Value.StimulusLocation;
		}
	}

	return bFoundLocation;
}

void USacraEnemyHatredComponent::ClearWarningTargetLocation()
{
	bHasWarningTargetLocation = false;
	CurrentWarningTargetLocation = FVector::ZeroVector;
}

void USacraEnemyHatredComponent::RememberLastKnownFightTargetLocation(const FVector& InLocation)
{
	bHasLastKnownFightTargetLocation = true;
	LastKnownFightTargetLocation = InLocation;
}

void USacraEnemyHatredComponent::ClearFightTargetActor()
{
	SetFightTargetActor(nullptr);
}

void USacraEnemyHatredComponent::CacheLastKnownLocationFromFightTarget()
{
	if (IsValid(CurrentFightTargetActor))
	{
		const FVector FightTargetLocation = CurrentFightTargetActor->GetActorLocation();
		CurrentWarningTargetLocation = FightTargetLocation;
		bHasWarningTargetLocation = true;
		RememberLastKnownFightTargetLocation(FightTargetLocation);
		return;
	}

	if (bHasLastKnownFightTargetLocation)
	{
		CurrentWarningTargetLocation = LastKnownFightTargetLocation;
		bHasWarningTargetLocation = true;
	}
}

void USacraEnemyHatredComponent::ClearHatredDecayTimers()
{
	if (!GetWorld())
	{
		return;
	}

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.ClearTimer(WarningStateDecayTimerHandle);
	TimerManager.ClearTimer(FightStateDecayTimerHandle);
}

void USacraEnemyHatredComponent::StartHatredValueUpdateTimer()
{
	if (!GetWorld() || bIsHatredPaused)
	{
		return;
	}

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.ClearTimer(UpdateHatredValueTimerHandle);
	TimerManager.SetTimer(
		UpdateHatredValueTimerHandle,
		this,
		&USacraEnemyHatredComponent::UpdateHatredValue,
		GetConfiguredUpdateHatredValueInterval(),
		true);

}

void USacraEnemyHatredComponent::UpdateWarningStateDecayTimer()
{
	if (bIsHatredPaused || CurrentHatredState != EHatredState::Warning || !GetWorld() || HasAnyPerception())
	{
		return;
	}

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	if (TimerManager.IsTimerActive(WarningStateDecayTimerHandle))
	{
		return;
	}

	TimerManager.ClearTimer(WarningStateDecayTimerHandle);
	TimerManager.SetTimer(
		WarningStateDecayTimerHandle,
		this,
		&USacraEnemyHatredComponent::HandleWarningStateDecayTimerExpired,
		GetConfiguredWarningStateDecayInterval(),
		false);

}

void USacraEnemyHatredComponent::UpdateFightStateDecayTimer()
{
	if (bIsHatredPaused || CurrentHatredState != EHatredState::Fight || !GetWorld() || HasAnyPerception())
	{
		return;
	}

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	if (TimerManager.IsTimerActive(FightStateDecayTimerHandle))
	{
		return;
	}

	TimerManager.ClearTimer(FightStateDecayTimerHandle);
	TimerManager.SetTimer(
		FightStateDecayTimerHandle,
		this,
		&USacraEnemyHatredComponent::HandleFightStateDecayTimerExpired,
		GetConfiguredFightStateDecayInterval(),
		false);

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy FightDecayTimer Started Owner=%s Interval=%.2f FightTarget=%s WarningLocation=%s"),
		*GetNameSafe(GetOwner()),
		GetConfiguredFightStateDecayInterval(),
		*GetNameSafe(CurrentFightTargetActor.Get()),
		bHasWarningTargetLocation ? *CurrentWarningTargetLocation.ToCompactString() : TEXT("None"));

}

void USacraEnemyHatredComponent::HandleWarningStateDecayTimerExpired()
{
	if (bIsHatredPaused || CurrentHatredState != EHatredState::Warning || HasAnyPerception())
	{
		return;
	}

	ClearWarningTargetLocation();
	CurrentHatredValue = 0.0f;
	ChangeHatredState(EHatredState::Idle);
	OnHatredValueChanged.Broadcast(CurrentHatredValue);
}

void USacraEnemyHatredComponent::HandleFightStateDecayTimerExpired()
{
	if (bIsHatredPaused || CurrentHatredState != EHatredState::Fight || HasAnyPerception())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy FightDecayTimer Expired Owner=%s FightTarget=%s LastKnownLocation=%s"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(CurrentFightTargetActor.Get()),
		bHasWarningTargetLocation ? *CurrentWarningTargetLocation.ToCompactString() : TEXT("None"));

	CacheLastKnownLocationFromFightTarget();
	ClearFightTargetActor();

	CurrentHatredValue = GetConfiguredWarningStateThreshold();
	ChangeHatredState(EHatredState::Warning);
	UpdateWarningStateDecayTimer();
	OnHatredValueChanged.Broadcast(CurrentHatredValue);
}
