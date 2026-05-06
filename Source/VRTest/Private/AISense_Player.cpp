// Fill out your copyright notice in the Description page of Project Settings.

#include "AISense_Player.h"

#include "AISenseConfig_Player.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Game/Characters/BasePlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISightTargetInterface.h"
#include "Perception/AIPerceptionComponent.h"

UAISense_Player::FDigestedPlayerProperties::FDigestedPlayerProperties(const UAISenseConfig_Player& SenseConfig)
{
	PlayerRadius = SenseConfig.PlayerRadius;
	PlayerSightDegree = SenseConfig.PlayerDegree;
	GrassSightRadius = SenseConfig.GrassSightRadius;
	bEnableDebugDraw = SenseConfig.bEnableDebugDraw;
	DebugDrawDuration = SenseConfig.DebugDrawDuration;
	DebugLineThickness = SenseConfig.DebugLineThickness;
	DebugVisibleColor = SenseConfig.DebugVisibleColor;
	DebugBlockedColor = SenseConfig.DebugBlockedColor;
	DebugRangeColor = SenseConfig.DebugRangeColor;
}

UAISense_Player::UAISense_Player(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	OnNewListenerDelegate.BindUObject(this, &UAISense_Player::OnNewListenerImpl);
	OnListenerUpdateDelegate.BindUObject(this, &UAISense_Player::OnListenerUpdateImpl);
	OnListenerRemovedDelegate.BindUObject(this, &UAISense_Player::OnListenerRemovedImpl);
}

float UAISense_Player::Update()
{
	const UWorld* World = GEngine->GetWorldFromContextObject(GetPerceptionSystem()->GetOuter(), EGetWorldErrorMode::LogAndReturnNull);
	if (World == nullptr)
	{
		return SuspendNextUpdate;
	}

	ABasePlayer* PlayerCharacter = ResolvePlayerCharacter(World);
	if (!IsValid(PlayerCharacter))
	{
		return SuspendNextUpdate;
	}

	AIPerception::FListenerMap& ListenersMap = *GetListeners();
	for (auto& ListenerPair : ListenersMap)
	{
		FPerceptionListener& Listener = ListenerPair.Value;
		const FPerceptionListenerID ListenerID = Listener.GetListenerID();
		FDigestedPlayerProperties* DigestedProperty = DigestedProperties.Find(ListenerID);
		if (DigestedProperty == nullptr)
		{
			continue;
		}

		const AActor* ListenerBodyActor = Listener.GetBodyActor();
		if (!IsValid(ListenerBodyActor))
		{
			continue;
		}

		float StimulusStrength = 0.0f;
		const bool bInRange = CheckTargetInRange(PlayerCharacter, StimulusStrength, Listener);

		FVector SeenLocation = PlayerCharacter->GetActorLocation();
		float OutSightStrength = 0.0f;
		const bool bHasLineOfSight = bInRange && PerformLineOfSightCheck(PlayerCharacter, Listener, SeenLocation, OutSightStrength);

		DrawDebugInfo(Listener, PlayerCharacter, bInRange, bHasLineOfSight, SeenLocation);

		if (bHasLineOfSight)
		{
			Listener.RegisterStimulus(PlayerCharacter, FAIStimulus(*this, StimulusStrength, SeenLocation, Listener.CachedLocation));
			DigestedProperty->bHasVisibleTarget = true;
			DigestedProperty->LastTargetActor = PlayerCharacter;
			DigestedProperty->LastTargetLocation = SeenLocation;
		}
		else if (DigestedProperty->bHasVisibleTarget && DigestedProperty->LastTargetActor.IsValid())
		{
			DigestedProperty->bHasVisibleTarget = false;
			Listener.RegisterStimulus(
				DigestedProperty->LastTargetActor.Get(),
				FAIStimulus(*this, -1.0f, DigestedProperty->LastTargetLocation, Listener.CachedLocation, FAIStimulus::SensingFailed));
		}
	}

	return 0.0f;
}

void UAISense_Player::OnNewListenerImpl(const FPerceptionListener& NewListener)
{
	UAIPerceptionComponent* NewListenerComponent = NewListener.Listener.Get();
	check(NewListenerComponent);

	const UAISenseConfig_Player* SenseConfig = Cast<const UAISenseConfig_Player>(NewListenerComponent->GetSenseConfig(GetSenseID()));
	check(SenseConfig);

	DigestedProperties.Add(NewListener.GetListenerID(), FDigestedPlayerProperties(*SenseConfig));
	RequestImmediateUpdate();
}

void UAISense_Player::OnListenerUpdateImpl(const FPerceptionListener& UpdatedListener)
{
	UAIPerceptionComponent* ListenerComponent = UpdatedListener.Listener.Get();
	if (!ListenerComponent)
	{
		return;
	}

	const UAISenseConfig_Player* SenseConfig = Cast<const UAISenseConfig_Player>(ListenerComponent->GetSenseConfig(GetSenseID()));
	if (!SenseConfig)
	{
		return;
	}

	DigestedProperties.FindOrAdd(UpdatedListener.GetListenerID()) = FDigestedPlayerProperties(*SenseConfig);
	RequestImmediateUpdate();
}

void UAISense_Player::OnListenerRemovedImpl(const FPerceptionListener& RemovedListener)
{
	DigestedProperties.Remove(RemovedListener.GetListenerID());
}

ABasePlayer* UAISense_Player::ResolvePlayerCharacter(const UWorld* World) const
{
	return World ? Cast<ABasePlayer>(UGameplayStatics::GetPlayerPawn(World, 0)) : nullptr;
}

bool UAISense_Player::CheckTargetInRange(const ABasePlayer* InTarget, float& OutStrength, const FPerceptionListener& Listener) const
{
	const FDigestedPlayerProperties* DigestedProperty = DigestedProperties.Find(Listener.GetListenerID());
	const AActor* ListenerBodyActor = Listener.GetBodyActor();
	if (!DigestedProperty || !IsValid(InTarget) || !IsValid(ListenerBodyActor))
	{
		return false;
	}

	const FVector PlayerLocation = InTarget->GetActorLocation();
	const FVector Direction = PlayerLocation - ListenerBodyActor->GetActorLocation();
	const float Distance = Direction.Size();

	const float EffectiveSightRadius = InTarget->GetTrackOrigin() ? DigestedProperty->PlayerRadius : DigestedProperty->GrassSightRadius;
	if (Distance > EffectiveSightRadius || EffectiveSightRadius <= 0.0f)
	{
		return false;
	}

	const float Dot = FVector::DotProduct(Direction.GetSafeNormal(), ListenerBodyActor->GetActorForwardVector());
	const float CheckAngle = FMath::Acos(FMath::Clamp(Dot, -1.0f, 1.0f));
	const float SightDegree = DigestedProperty->PlayerSightDegree;

	if (CheckAngle <= SightDegree)
	{
		OutStrength = (-9.0f * FMath::Square(Distance / EffectiveSightRadius) + 10.0f) * FMath::Cos(CheckAngle);
		return true;
	}

	if (CheckAngle >= PI / 2.0f && CheckAngle <= PI * 5.0f / 6.0f && Distance <= DigestedProperty->GrassSightRadius)
	{
		OutStrength = 4.0f;
		return true;
	}

	if (CheckAngle > PI * 5.0f / 6.0f && Distance <= DigestedProperty->GrassSightRadius)
	{
		OutStrength = 2.0f;
		return true;
	}

	if (CheckAngle > SightDegree && CheckAngle < PI / 2.0f && Distance <= EffectiveSightRadius * 0.5f)
	{
		OutStrength = (-9.0f * FMath::Square(Distance / EffectiveSightRadius) + 10.0f) * FMath::Cos(CheckAngle);
		return true;
	}

	return false;
}

bool UAISense_Player::PerformLineOfSightCheck(const ABasePlayer* TargetPlayer, const FPerceptionListener& Listener, FVector& OutSeenLocation, float& OutSightStrength) const
{
	if (!IsValid(TargetPlayer))
	{
		return false;
	}

	if (const IAISightTargetInterface* SightTarget = Cast<const IAISightTargetInterface>(TargetPlayer))
	{
		int32 NumberOfLoSChecksPerformed = 0;
		return SightTarget->CanBeSeenFrom(
			Listener.CachedLocation,
			OutSeenLocation,
			NumberOfLoSChecksPerformed,
			OutSightStrength,
			Listener.GetBodyActor());
	}

	const UCameraComponent* CameraComponent = TargetPlayer->FindComponentByClass<UCameraComponent>();
	OutSeenLocation = CameraComponent ? CameraComponent->GetComponentLocation() : TargetPlayer->GetActorLocation();

	FHitResult HitResult;
	const bool bHit = TargetPlayer->GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Listener.CachedLocation,
		OutSeenLocation,
		ECC_Visibility,
		FCollisionQueryParams(TEXT("SacraPlayerSenseLoS"), false, Listener.GetBodyActor()));

	const bool bVisible = !bHit || (HitResult.GetActor() && HitResult.GetActor()->IsOwnedBy(TargetPlayer));
	OutSightStrength = bVisible ? 1.0f : 0.0f;
	return bVisible;
}

void UAISense_Player::DrawDebugInfo(const FPerceptionListener& Listener, const ABasePlayer* TargetPlayer, bool bInRange, bool bHasLineOfSight, const FVector& SeenLocation) const
{
	const FDigestedPlayerProperties* DigestedProperty = DigestedProperties.Find(Listener.GetListenerID());
	const AActor* ListenerBodyActor = Listener.GetBodyActor();
	if (!DigestedProperty || !DigestedProperty->bEnableDebugDraw || !IsValid(TargetPlayer) || !IsValid(ListenerBodyActor))
	{
		return;
	}

	UWorld* World = ListenerBodyActor->GetWorld();
	if (!World)
	{
		return;
	}

	const float Duration = DigestedProperty->DebugDrawDuration;
	const float Thickness = DigestedProperty->DebugLineThickness;
	const FVector ListenerLocation = ListenerBodyActor->GetActorLocation();
	const FColor TraceColor = bHasLineOfSight ? DigestedProperty->DebugVisibleColor : DigestedProperty->DebugBlockedColor;

	DrawDebugLine(World, ListenerLocation, SeenLocation, TraceColor, false, Duration, 0, Thickness);
	DrawDebugSphere(World, SeenLocation, 12.0f, 8, TraceColor, false, Duration);
	DrawDebugCylinder(World, ListenerLocation, ListenerLocation + FVector(0.0f, 0.0f, 25.0f), DigestedProperty->PlayerRadius, 24, DigestedProperty->DebugRangeColor, false, Duration, 0, Thickness);

	if (!bInRange)
	{
		DrawDebugSphere(World, TargetPlayer->GetActorLocation(), 24.0f, 8, DigestedProperty->DebugBlockedColor, false, Duration);
	}
}
