// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Component/SacraBlackboardComponent.h"

#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Enum.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

namespace
{
const TCHAR* LexToStringHatredStateBlackboard(const EHatredState InState)
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

USacraBlackboardComponent::USacraBlackboardComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USacraBlackboardComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!HasValidAsset() && DefaultBlackboardAsset)
	{
		InitializeBlackboard(*DefaultBlackboardAsset);
	}
}

void USacraBlackboardComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindHatredDelegates();

	Super::EndPlay(EndPlayReason);
}

void USacraBlackboardComponent::InitAutoCollect(USacraEnemyHatredComponent* InHatredComponent)
{
	if (CachedHatredComponent == InHatredComponent)
	{
		RefreshAutoCollectedKeys();
		return;
	}

	UnbindHatredDelegates();
	CachedHatredComponent = InHatredComponent;
	if (!bIsAutoCollectPaused)
	{
		BindHatredDelegates();
	}

	RefreshAutoCollectedKeys();
}

void USacraBlackboardComponent::RefreshAutoCollectedKeys()
{
	if (bIsAutoCollectPaused || !IsValid(CachedHatredComponent) || !HasValidAsset())
	{
		return;
	}

	SyncHatredState();
	SyncHatredValue();
	SyncHatredTargets();
}

void USacraBlackboardComponent::SetAutoCollectPaused(bool bInPaused)
{
	if (bIsAutoCollectPaused == bInPaused)
	{
		return;
	}

	bIsAutoCollectPaused = bInPaused;

	if (bIsAutoCollectPaused)
	{
		UnbindHatredDelegates();
		return;
	}

	UnbindHatredDelegates();
	BindHatredDelegates();
	RefreshAutoCollectedKeys();
}

void USacraBlackboardComponent::BindHatredDelegates()
{
	if (!IsValid(CachedHatredComponent))
	{
		return;
	}

	CachedHatredComponent->OnHatredStateChanged.AddDynamic(this, &USacraBlackboardComponent::HandleHatredStateChanged);
	CachedHatredComponent->OnHatredValueChanged.AddDynamic(this, &USacraBlackboardComponent::HandleHatredValueChanged);
}

void USacraBlackboardComponent::UnbindHatredDelegates()
{
	if (!IsValid(CachedHatredComponent))
	{
		return;
	}

	CachedHatredComponent->OnHatredStateChanged.RemoveDynamic(this, &USacraBlackboardComponent::HandleHatredStateChanged);
	CachedHatredComponent->OnHatredValueChanged.RemoveDynamic(this, &USacraBlackboardComponent::HandleHatredValueChanged);
}

void USacraBlackboardComponent::SyncHatredState()
{
	if (!IsValid(CachedHatredComponent))
	{
		return;
	}

	SetEnumIfKeyExists(HatredStateKeyName, static_cast<uint8>(CachedHatredComponent->GetCurrentHatredState()));
}

void USacraBlackboardComponent::SyncHatredValue()
{
	if (!IsValid(CachedHatredComponent))
	{
		return;
	}

	const float HatredValue = CachedHatredComponent->GetCurrentHatredValue();
	const float MaxHatredValue = FMath::Max(CachedHatredComponent->GetMaxHatredValue(), 1.0f);

	SetFloatIfKeyExists(HatredValueKeyName, HatredValue);
	SetFloatIfKeyExists(HatredPercentKeyName, HatredValue / MaxHatredValue);
}

void USacraBlackboardComponent::SyncHatredTargets()
{
	if (!IsValid(CachedHatredComponent))
	{
		return;
	}

	const bool bHasWarningLocation = CachedHatredComponent->HasWarningTargetLocation();
	SetBoolIfKeyExists(HasWarningLocationKeyName, bHasWarningLocation);
	if (bHasWarningLocation)
	{
		SetVectorIfKeyExists(WarningLocationKeyName, CachedHatredComponent->GetCurrentWarningTargetLocation());
	}
	else
	{
		ClearValueIfKeyExists(WarningLocationKeyName);
	}

	AActor* FightTargetActor = CachedHatredComponent->GetCurrentFightTargetActor();
	const bool bHasFightTarget = IsValid(FightTargetActor);
	SetBoolIfKeyExists(HasFightTargetKeyName, bHasFightTarget);
	if (bHasFightTarget)
	{
		SetObjectIfKeyExists(FightTargetKeyName, FightTargetActor);
	}
	else
	{
		ClearValueIfKeyExists(FightTargetKeyName);
	}
}

void USacraBlackboardComponent::SetEnumIfKeyExists(const FName& KeyName, uint8 InValue)
{
	if (GetKeyID(KeyName) != FBlackboard::InvalidKey)
	{
		SetValueAsEnum(KeyName, InValue);
	}
}

void USacraBlackboardComponent::SetFloatIfKeyExists(const FName& KeyName, float InValue)
{
	if (GetKeyID(KeyName) != FBlackboard::InvalidKey)
	{
		SetValueAsFloat(KeyName, InValue);
	}
}

void USacraBlackboardComponent::SetBoolIfKeyExists(const FName& KeyName, bool bInValue)
{
	if (GetKeyID(KeyName) != FBlackboard::InvalidKey)
	{
		SetValueAsBool(KeyName, bInValue);
	}
}

void USacraBlackboardComponent::SetVectorIfKeyExists(const FName& KeyName, const FVector& InValue)
{
	if (GetKeyID(KeyName) != FBlackboard::InvalidKey)
	{
		SetValueAsVector(KeyName, InValue);
	}
}

void USacraBlackboardComponent::SetObjectIfKeyExists(const FName& KeyName, UObject* InValue)
{
	if (GetKeyID(KeyName) != FBlackboard::InvalidKey)
	{
		SetValueAsObject(KeyName, InValue);
	}
}

void USacraBlackboardComponent::ClearValueIfKeyExists(const FName& KeyName)
{
	if (GetKeyID(KeyName) != FBlackboard::InvalidKey)
	{
		ClearValue(KeyName);
	}
}

void USacraBlackboardComponent::HandleHatredStateChanged(EHatredState NewState)
{
	if (bIsAutoCollectPaused)
	{
		return;
	}

	SyncHatredState();
	SyncHatredTargets();

	const bool bHasWarningLocation = CachedHatredComponent->HasWarningTargetLocation();
	const FString WarningLocationText = bHasWarningLocation
		? CachedHatredComponent->GetCurrentWarningTargetLocation().ToCompactString()
		: TEXT("None");
	AActor* FightTargetActor = CachedHatredComponent->GetCurrentFightTargetActor();
	const bool bHasFightTarget = IsValid(FightTargetActor);

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy Blackboard HatredState Synced Owner=%s State=%s HasWarningLocation=%s WarningLocation=%s HasFightTarget=%s FightTarget=%s"),
		*GetNameSafe(GetOwner()),
		LexToStringHatredStateBlackboard(NewState),
		bHasWarningLocation ? TEXT("true") : TEXT("false"),
		*WarningLocationText,
		bHasFightTarget ? TEXT("true") : TEXT("false"),
		*GetNameSafe(FightTargetActor));
}

void USacraBlackboardComponent::HandleHatredValueChanged(float NewValue)
{
	if (bIsAutoCollectPaused)
	{
		return;
	}

	SyncHatredValue();
	SyncHatredTargets();
}
