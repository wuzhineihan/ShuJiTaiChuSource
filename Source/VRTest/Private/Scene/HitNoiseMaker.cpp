// Fill out your copyright notice in the Description page of Project Settings.

#include "Scene/HitNoiseMaker.h"

#include "Audio/AudioSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Game/CollisionConfig.h"
#include "Game/MyGameplayTags.h"
#include "Kismet/GameplayStatics.h"

AHitNoiseMaker::AHitNoiseMaker()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SM_Tank"));
	SetRootComponent(Mesh);

	// Prefer collision profile (configured in Project Settings -> Collision).
	Mesh->SetCollisionProfileName(CP_HIT_NOISE_MAKER);
	Mesh->SetGenerateOverlapEvents(false);
	Mesh->SetSimulatePhysics(false);

	// Default sound tag for this actor.
	HitSoundTag = MyProjectTags::TAG_NormalSound_HitNoise;
}

void AHitNoiseMaker::BeginPlay()
{
	Super::BeginPlay();
}

void AHitNoiseMaker::ApplyEffect_Implementation(const FEffect& Effect)
{
	TriggerNoise();
}

void AHitNoiseMaker::TriggerNoise()
{
	UE_LOG(LogTemp, Warning, TEXT("TriggerNoiseMaker"));
	const FVector Location = GetActorLocation();

	APawn* InstigatorPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	MakeNoise(NoiseLoudness, InstigatorPawn, Location, NoiseRange);

	if (HitSoundTag.IsValid())
	{
		if (UAudioSubsystem* AudioSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAudioSubsystem>() : nullptr)
		{
			AudioSubsystem->PlayNormalSoundAtLocation(HitSoundTag, Location);
		}
	}
}

