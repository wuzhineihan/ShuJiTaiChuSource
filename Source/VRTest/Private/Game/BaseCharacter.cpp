// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BaseCharacter.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AliveComponent = CreateDefaultSubobject<UAliveComponent>(TEXT("AliveComponent"));
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (AliveComponent)
	{
		AliveComponent->OnDead.AddDynamic(this, &ABaseCharacter::OnDeath);
	}
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

USceneComponent* ABaseCharacter::GetTrackOrigin() const
{
	return GetRootComponent();
}

void ABaseCharacter::ApplyEffect_Implementation(const FEffect& Effect)
{
	for (const EEffectType& Type : Effect.EffectTypes)
	{
		switch (Type)
		{
		case EEffectType::Arrow:
			TakeArrowEffect(Effect);
			break;
		case EEffectType::Smash:
			TakeSmashEffect(Effect);
			break;
		case EEffectType::Melee:
			TakeMeleeEffect(Effect);
			break;
		case EEffectType::Fire:
			TakeFireEffect(Effect);
			break;
		case EEffectType::Stasis:
			TakeStasisEffect(Effect);
			break;
		default:
			break;
		}
	}
}

void ABaseCharacter::TakeArrowEffect(const FEffect& Effect)
{
	// Default implementation
}

void ABaseCharacter::TakeSmashEffect(const FEffect& Effect)
{
	// Default implementation
}

void ABaseCharacter::TakeMeleeEffect(const FEffect& Effect)
{
	// Default implementation
}

void ABaseCharacter::TakeFireEffect(const FEffect& Effect)
{
	// Default implementation
}

void ABaseCharacter::TakeStasisEffect(const FEffect& Effect)
{
	// Default implementation
}

void ABaseCharacter::OnDeath_Implementation()
{
	// Default implementation
}
