// Fill out your copyright notice in the Description page of Project Settings.

#include "Scene/HangedCargo.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Effect/EffectTypes.h"
#include "Effect/Effectable.h"
#include "Game/CollisionConfig.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

AHangedCargo::AHangedCargo()
{
	PrimaryActorTick.bCanEverTick = false;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	SetRootComponent(DefaultSceneRoot);

	InvisibleBlock = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InvisibleBlock"));
	InvisibleBlock->SetupAttachment(DefaultSceneRoot);
	InvisibleBlock->SetSimulatePhysics(false);
	InvisibleBlock->SetCollisionProfileName(FName(TEXT("BlockAll")));
	InvisibleBlock->SetVisibility(false);

	Rope = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rope"));
	Rope->SetupAttachment(DefaultSceneRoot);
	Rope->SetSimulatePhysics(true);
	Rope->SetCollisionProfileName(CP_ROPE);

	RopeCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RopeCollision"));
	RopeCollision->SetupAttachment(Rope);
	RopeCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RopeCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	RopeCollision->SetCollisionResponseToChannel(TCC_PROJECTILE, ECR_Block);
	RopeCollision->SetBoxExtent(FVector(10.0f, 10.0f, 100.0f));
	RopeCollision->ComponentTags.AddUnique(FName(TEXT("ArrowPassthrough")));

	Cargo = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cargo"));
	Cargo->SetupAttachment(DefaultSceneRoot);
	Cargo->SetSimulatePhysics(true);

	PhysicsConstraintUp = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PhysicsConstraintUp"));
	PhysicsConstraintUp->SetupAttachment(DefaultSceneRoot);
	PhysicsConstraintUp->SetConstrainedComponents(InvisibleBlock, NAME_None, Rope, NAME_None);
	PhysicsConstraintUp->SetDisableCollision(true);
	PhysicsConstraintUp->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Limited, 100.0f);

	PhysicsConstraintDown = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PhysicsConstraintDown"));
	PhysicsConstraintDown->SetupAttachment(DefaultSceneRoot);
	PhysicsConstraintDown->SetConstrainedComponents(Rope, NAME_None, Cargo, NAME_None);
	PhysicsConstraintDown->SetDisableCollision(true);
	PhysicsConstraintDown->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Limited, 100.0f);
}

void AHangedCargo::BeginPlay()
{
	Super::BeginPlay();

	if (Cargo)
	{
		Cargo->OnComponentHit.AddDynamic(this, &AHangedCargo::OnCargoHit);
	}
}

void AHangedCargo::OnArrowPassThrough_Implementation(AActor* Arrow)
{
	CutRope();
}

void AHangedCargo::CutRope()
{
	if (bRopeCut)
	{
		return;
	}
	bRopeCut = true;

	if (PhysicsConstraintUp)
	{
		PhysicsConstraintUp->BreakConstraint();
	}
	if (PhysicsConstraintDown)
	{
		PhysicsConstraintDown->BreakConstraint();
	}
	if (Cargo)
	{
		Cargo->WakeRigidBody();
	}
	if (Rope)
	{
		Rope->SetSimulatePhysics(false);
		Rope->SetVisibility(false);
	}
}

void AHangedCargo::OnCargoHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!bRopeCut || !OtherActor || OtherActor == this)
	{
		return;
	}

	const float ImpactSpeed = Cargo ? Cargo->GetPhysicsLinearVelocity().Size() : 0.0f;
	if (ImpactSpeed < MinImpactVelocity)
	{
		return;
	}

	FEffect SmashEffect;
	SmashEffect.EffectTypes.Add(EEffectType::Smash);
	SmashEffect.Amount = CargoDamage;
	SmashEffect.Causer = this;
	SmashEffect.Duration = 0.0f;

	if (IEffectable* Effectable = Cast<IEffectable>(OtherActor))
	{
		IEffectable::Execute_ApplyEffect(OtherActor, SmashEffect);
	}
}
