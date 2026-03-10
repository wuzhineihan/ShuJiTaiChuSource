// Fill out your copyright notice in the Description page of Project Settings.

#include "Scene/FireObject.h"

#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Effect/EffectTypes.h"
#include "Effect/Effectable.h"
#include "Game/CollisionConfig.h"
#include "NiagaraComponent.h"

AFireObject::AFireObject()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	FireLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FireLight"));
	FireLight->SetupAttachment(Mesh);

	FireNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FireNiagara"));
	FireNiagara->SetupAttachment(Mesh);
	FireNiagara->SetAutoActivate(true);

	FireSphere = CreateDefaultSubobject<USphereComponent>(TEXT("FireSphere"));
	FireSphere->SetupAttachment(Mesh);
	FireSphere->InitSphereRadius(60.0f);
	FireSphere->SetCollisionProfileName(CP_FIRE_OBJECT_SPHERE);
	FireSphere->SetGenerateOverlapEvents(true);
	FireSphere->OnComponentBeginOverlap.AddDynamic(this, &AFireObject::OnFireSphereBeginOverlap);
}

void AFireObject::BeginPlay()
{
	Super::BeginPlay();

	if (FireSphere && !IgniteBySightComponentTag.IsNone() && !FireSphere->ComponentHasTag(IgniteBySightComponentTag))
	{
		FireSphere->ComponentTags.Add(IgniteBySightComponentTag);
	}
}

void AFireObject::OnFireSphereBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	if (!OtherActor->Implements<UEffectable>())
	{
		return;
	}

	FEffect FireEffect;
	FireEffect.EffectTypes.Add(EEffectType::Fire);
	FireEffect.Amount = FireEffectAmount;
	FireEffect.Duration = FireEffectDuration;
	FireEffect.Causer = this;
	FireEffect.Instigator = nullptr;

	IEffectable::Execute_ApplyEffect(OtherActor, FireEffect);
}
