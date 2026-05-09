// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/Characters/SacraEnemy.h"

#include "AI/Component/SacraEnemyContextComponent.h"
#include "AI/DataAsset/SacraEnemyConfigDataAsset.h"
#include "AI/SacraEnemyAIControllerBase.h"
#include "AI/Component/SacraEnemyLoadoutComponent.h"
#include "AI/Component/SacraEnemyStatusUIComponent.h"
#include "AI/Component/SacraEnemyWeaponComponent.h"
#include "AI/Component/SacraMeleeWeaponComponent.h"

ASacraEnemy::ASacraEnemy()
{
	EnemyContextComponent = CreateDefaultSubobject<USacraEnemyContextComponent>(TEXT("EnemyContextComponent"));
	EnemyStatusUIComponent = CreateDefaultSubobject<USacraEnemyStatusUIComponent>(TEXT("EnemyStatusUIComponent"));
	EnemyLoadoutComponent = CreateDefaultSubobject<USacraEnemyLoadoutComponent>(TEXT("EnemyLoadoutComponent"));
	EnemyWeaponComponentClass = USacraMeleeWeaponComponent::StaticClass();
}

void ASacraEnemy::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	ApplyEnemyConfigDataAsset();
	EnsureWeaponComponent();
}

void ASacraEnemy::ApplyEnemyConfigDataAsset()
{
	if (!IsValid(EnemyConfigDataAsset))
	{
		return;
	}

	if (EnemyConfigDataAsset->GeneralConfig.bOverrideWeaponComponentClass
		&& EnemyConfigDataAsset->GeneralConfig.EnemyWeaponComponentClass)
	{
		EnemyWeaponComponentClass = EnemyConfigDataAsset->GeneralConfig.EnemyWeaponComponentClass;
	}

	if (IsValid(EnemyContextComponent))
	{
		EnemyContextComponent->ApplyConfigData(EnemyConfigDataAsset->ContextConfig);
	}

	if (IsValid(EnemyStatusUIComponent))
	{
		EnemyStatusUIComponent->ApplyConfigData(EnemyConfigDataAsset->StatusUIConfig);
	}

	if (IsValid(EnemyLoadoutComponent))
	{
		EnemyLoadoutComponent->ApplyConfigData(EnemyConfigDataAsset->LoadoutConfig);
	}

	if (ASacraEnemyAIControllerBase* EnemyController = Cast<ASacraEnemyAIControllerBase>(GetController()))
	{
		EnemyController->ApplyConfigDataAsset(EnemyConfigDataAsset);
	}
}

void ASacraEnemy::EnsureWeaponComponent()
{
	if (IsValid(EnemyWeaponComponent))
	{
		return;
	}

	TArray<USacraEnemyWeaponComponent*> ExistingWeaponComponents;
	GetComponents<USacraEnemyWeaponComponent>(ExistingWeaponComponents);

	if (EnemyWeaponComponentClass)
	{
		for (USacraEnemyWeaponComponent* ExistingWeaponComponent : ExistingWeaponComponents)
		{
			if (IsValid(ExistingWeaponComponent) && ExistingWeaponComponent->IsA(EnemyWeaponComponentClass))
			{
				EnemyWeaponComponent = ExistingWeaponComponent;
				return;
			}
		}
	}

	for (USacraEnemyWeaponComponent* ExistingWeaponComponent : ExistingWeaponComponents)
	{
		if (IsValid(ExistingWeaponComponent))
		{
			EnemyWeaponComponent = ExistingWeaponComponent;
			return;
		}
	}

	if (!EnemyWeaponComponentClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("SacraEnemy MissingWeaponComponentClass Owner=%s"), *GetNameSafe(this));
		return;
	}

	const FName ComponentName = MakeUniqueObjectName(this, EnemyWeaponComponentClass, TEXT("EnemyWeaponComponent"));
	EnemyWeaponComponent = NewObject<USacraEnemyWeaponComponent>(this, EnemyWeaponComponentClass, ComponentName);
	if (!IsValid(EnemyWeaponComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("SacraEnemy CreateWeaponComponent Failed Owner=%s Class=%s"),
			*GetNameSafe(this),
			*GetNameSafe(EnemyWeaponComponentClass));
		return;
	}

	EnemyWeaponComponent->CreationMethod = EComponentCreationMethod::Instance;
	AddInstanceComponent(EnemyWeaponComponent);
	EnemyWeaponComponent->RegisterComponent();

	UE_LOG(LogTemp, Log, TEXT("SacraEnemy CreateWeaponComponent Success Owner=%s Class=%s Component=%s"),
		*GetNameSafe(this),
		*GetNameSafe(EnemyWeaponComponentClass),
		*GetNameSafe(EnemyWeaponComponent));
}
