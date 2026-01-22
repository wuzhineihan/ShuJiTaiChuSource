// Fill out your copyright notice in the Description page of Project Settings.

#include "Grabbee/BreakableGrabbeeObject.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Grabber/PlayerGrabHand.h"
#include "Kismet/GameplayStatics.h"
#include "Game/CollisionConfig.h"
#include "Grabbee/Arrow.h"

ABreakableGrabbeeObject::ABreakableGrabbeeObject()
{
	PrimaryActorTick.bCanEverTick = true;

	// Intact state: treat as a normal physics grabbable.
	if (MeshComponent)
	{
		MeshComponent->SetCollisionProfileName(FName("Profile_Grabbable_Physics"));
		MeshComponent->SetNotifyRigidBodyCollision(true); // required for OnComponentHit
		MeshComponent->OnComponentHit.AddDynamic(this, &ABreakableGrabbeeObject::OnMeshHit);
	}

	GeometryCollection = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollection"));
	GeometryCollection->SetupAttachment(MeshComponent);
	GeometryCollection->SetVisibility(false, true);
	GeometryCollection->SetSimulatePhysics(false);
	GeometryCollection->SetCollisionProfileName(CP_NO_COLLISION);
	
}

void ABreakableGrabbeeObject::OnMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                        UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bBroken || !OtherComp)
	{
		return;
	}
	
	if (NormalImpulse.Size() > BreakImpulseThreshold)
	{
		Break();
	}
}

void ABreakableGrabbeeObject::SpawnForceField_Implementation(FTransform SpawnTransform)
{
	//Blueprint Implemented
}

void ABreakableGrabbeeObject::ApplyEffect_Implementation(const FEffect& Effect)
{
	if (bBroken)
	{
		return;
	}

	bool bShouldBreak = false;
	for (const EEffectType Type : Effect.EffectTypes)
	{
		if (Type == EEffectType::Arrow)
		{
			bShouldBreak = true;
			if (AArrow* Arrow = Cast<AArrow>(Effect.Causer))
			{
				Arrow->EnterIdleState();
			}
				
			break;
		}
		if (Effect.Amount > SmashDamageThreshold)
		{
			if (Type == EEffectType::Smash || Type == EEffectType::Melee)
			{
				bShouldBreak = true;
				break;
			}
		}
	}

	if (bShouldBreak)
	{
		Break();
	}
}

void ABreakableGrabbeeObject::Break()
{
	if (bBroken)
	{
		return;
	}
	bBroken = true;

	// If currently held, force a real release so we don't keep a physics handle on a hidden mesh.
	if (HoldingHand && bIsHeld)
	{
		HoldingHand->ReleaseObject();
	}

	// Disable grabbing once broken (match BP: Set CanGrab=false).
	bCanGrab = false;

	// Swap visuals/physics: hide intact mesh, enable geometry collection.
	if (MeshComponent)
	{
		MeshComponent->SetVisibility(false, true);
		MeshComponent->SetSimulatePhysics(false);
		MeshComponent->SetCollisionProfileName(CP_NO_COLLISION);
	}

	if (GeometryCollection)
	{
		GeometryCollection->SetVisibility(true, true);
		GeometryCollection->SetCollisionProfileName(CP_DEBRIS);
		GeometryCollection->SetSimulatePhysics(true);
	}

	FVector Loc = GeometryCollection->GetCenterOfMass();
	SpawnForceField(FTransform(Loc));

	// AI noise (approx. BP MakeNoise call).
	if (NoiseRange > 0.0f)
	{
		APawn* InstigatorPawn = UGameplayStatics::GetPlayerPawn(this, 0);
		MakeNoise(1.0f, InstigatorPawn, GetActorLocation(), NoiseRange);
	}

	SetLifeSpan(DestroyDelaySeconds);
}
