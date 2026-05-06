// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Weapon/SacraEnemyBowActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Game/CollisionConfig.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

ASacraEnemyBowActor::ASacraEnemyBowActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(RootSceneComponent);

	StandardPoint = CreateDefaultSubobject<USceneComponent>(TEXT("StandardPoint"));
	StandardPoint->SetupAttachment(RootSceneComponent);

	BowMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponBow"));
	BowMeshComponent->SetupAttachment(RootSceneComponent);
	BowMeshComponent->SetCollisionProfileName(CP_NO_COLLISION);

	StringMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponString"));
	StringMeshComponent->SetupAttachment(BowMeshComponent);
	StringMeshComponent->SetCollisionProfileName(CP_NO_COLLISION);
}

void ASacraEnemyBowActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	EnsureStringMaterialInstance();
}

void ASacraEnemyBowActor::SetStringPullState(float InPullAlpha, const FVector& InGrabLocation)
{
	EnsureStringMaterialInstance();
	if (!IsValid(StringMaterialInstance))
	{
		return;
	}

	StringMaterialInstance->SetScalarParameterValue(TEXT("Grabbed"), InPullAlpha);
	StringMaterialInstance->SetVectorParameterValue(TEXT("GrabSpot"), FLinearColor(InGrabLocation));
}

void ASacraEnemyBowActor::EnsureStringMaterialInstance()
{
	if (!IsValid(StringMeshComponent) || IsValid(StringMaterialInstance))
	{
		return;
	}

	UMaterialInterface* MaterialToUse = StringMaterial.Get() ? StringMaterial.Get() : StringMeshComponent->GetMaterial(0);
	if (!IsValid(MaterialToUse))
	{
		return;
	}

	StringMaterialInstance = StringMeshComponent->CreateDynamicMaterialInstance(0, MaterialToUse);
}
