// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Component/SacraEnemyContextComponent.h"

#include "AI/Component/SacraBlackboardComponent.h"
#include "AI/Component/SacraEnemyHatredComponent.h"
#include "AI/Component/SacraEnemyWeaponComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnemyPatrolSplineComponent.h"
#include "GameFramework/Pawn.h"
#include "Game/Characters/SacraEnemy.h"

USacraEnemyContextComponent::USacraEnemyContextComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USacraEnemyContextComponent::BeginPlay()
{
	Super::BeginPlay();

	CacheSpawnTransform();

	if (bAutoResolveSharedReferences)
	{
		ResolveSharedReferences();
	}
}

AAIController* USacraEnemyContextComponent::GetCachedAIController() const
{
	USacraEnemyContextComponent* MutableThis = const_cast<USacraEnemyContextComponent*>(this);
	MutableThis->ResolveAIController();
	return CachedAIController.Get();
}

USacraBlackboardComponent* USacraEnemyContextComponent::GetCachedSacraBlackboardComponent() const
{
	USacraEnemyContextComponent* MutableThis = const_cast<USacraEnemyContextComponent*>(this);
	MutableThis->ResolveBlackboardComponent();
	return CachedSacraBlackboardComponent.Get();
}

UBlackboardComponent* USacraEnemyContextComponent::GetCachedBlackboardComponent() const
{
	USacraEnemyContextComponent* MutableThis = const_cast<USacraEnemyContextComponent*>(this);
	MutableThis->ResolveBlackboardComponent();
	return CachedBlackboardComponent.Get();
}

USacraEnemyHatredComponent* USacraEnemyContextComponent::GetCachedHatredComponent() const
{
	USacraEnemyContextComponent* MutableThis = const_cast<USacraEnemyContextComponent*>(this);
	MutableThis->ResolveHatredComponent();
	return CachedHatredComponent.Get();
}

USacraEnemyWeaponComponent* USacraEnemyContextComponent::GetCachedWeaponComponent() const
{
	USacraEnemyContextComponent* MutableThis = const_cast<USacraEnemyContextComponent*>(this);
	MutableThis->ResolveWeaponComponent();
	return CachedWeaponComponent.Get();
}

UEnemyPatrolSplineComponent* USacraEnemyContextComponent::GetPatrolSplineComponent() const
{
	USacraEnemyContextComponent* MutableThis = const_cast<USacraEnemyContextComponent*>(this);
	MutableThis->ResolvePatrolSplineComponent();
	return CachedPatrolSplineComponent.Get();
}

FVector USacraEnemyContextComponent::GetStandLocation() const
{
	return bUseSpawnTransformAsStandTransform ? CachedSpawnLocation : StandLocation;
}

FRotator USacraEnemyContextComponent::GetStandRotation() const
{
	return bUseSpawnTransformAsStandTransform ? CachedSpawnRotation : StandRotation;
}

bool USacraEnemyContextComponent::HasPatrolRoute() const
{
	const_cast<USacraEnemyContextComponent*>(this)->ResolvePatrolSplineComponent();

	return bEnablePatrol
		&& IsValid(CachedPatrolSplineComponent)
		&& CachedPatrolSplineComponent->HasPatrolPoints();
}

bool USacraEnemyContextComponent::TryGetCurrentPatrolPoint(FVector& OutPatrolPointLocation) const
{
	if (!HasPatrolRoute())
	{
		return false;
	}

	OutPatrolPointLocation = CachedPatrolSplineComponent->GetCurrentPatrolPointLocation();
	return true;
}

bool USacraEnemyContextComponent::AdvancePatrolPoint()
{
	if (!HasPatrolRoute())
	{
		return false;
	}

	return CachedPatrolSplineComponent->AdvancePatrolPoint();
}

void USacraEnemyContextComponent::SetCachedWarningSearchLocation(const FVector& InSearchLocation)
{
	bHasCachedWarningSearchLocation = true;
	CachedWarningSearchLocation = InSearchLocation;
}

void USacraEnemyContextComponent::ClearCachedWarningSearchLocation()
{
	bHasCachedWarningSearchLocation = false;
	CachedWarningSearchLocation = FVector::ZeroVector;
}

void USacraEnemyContextComponent::SetCachedWarningAnchorLocation(const FVector& InAnchorLocation)
{
	bHasCachedWarningAnchorLocation = true;
	CachedWarningAnchorLocation = InAnchorLocation;
}

void USacraEnemyContextComponent::ClearCachedWarningAnchorLocation()
{
	bHasCachedWarningAnchorLocation = false;
	CachedWarningAnchorLocation = FVector::ZeroVector;
}

void USacraEnemyContextComponent::ApplyConfigData(const FSacraEnemyContextConfig& ConfigData)
{
	if (!ConfigData.bOverrideContextConfig)
	{
		return;
	}

	bAutoResolveSharedReferences = ConfigData.bAutoResolveSharedReferences;
	bUseSpawnTransformAsStandTransform = ConfigData.bUseSpawnTransformAsStandTransform;
	StandLocation = ConfigData.StandLocation;
	StandRotation = ConfigData.StandRotation;
	bEnablePatrol = ConfigData.bEnablePatrol;
	IdleMoveSpeed = ConfigData.IdleMoveSpeed;
	PatrolMoveSpeed = ConfigData.PatrolMoveSpeed;
	WarningMoveSpeed = ConfigData.WarningMoveSpeed;
	WarningSearchRadius = ConfigData.WarningSearchRadius;
	WarningSearchPointCount = ConfigData.WarningSearchPointCount;
	WarningSearchReachableRadius = ConfigData.WarningSearchReachableRadius;
	bEnableWarningSupportRequest = ConfigData.bEnableWarningSupportRequest;
	WarningSupportRequestRadius = ConfigData.WarningSupportRequestRadius;
}

void USacraEnemyContextComponent::CacheSpawnTransform()
{
	if (AActor* OwnerActor = GetOwner())
	{
		CachedSpawnLocation = OwnerActor->GetActorLocation();
		CachedSpawnRotation = OwnerActor->GetActorRotation();
	}
}

void USacraEnemyContextComponent::ResolveSharedReferences()
{
	ResolveAIController();
	ResolveBlackboardComponent();
	ResolveHatredComponent();
	ResolveWeaponComponent();
	ResolvePatrolSplineComponent();
}

void USacraEnemyContextComponent::ResolveAIController()
{
	if (IsValid(CachedAIController))
	{
		return;
	}

	if (const APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		CachedAIController = Cast<AAIController>(OwnerPawn->GetController());
	}
}

void USacraEnemyContextComponent::ResolveBlackboardComponent()
{
	if (IsValid(CachedBlackboardComponent))
	{
		return;
	}

	ResolveAIController();
	if (!IsValid(CachedAIController))
	{
		return;
	}

	CachedSacraBlackboardComponent = CachedAIController->FindComponentByClass<USacraBlackboardComponent>();
	CachedBlackboardComponent = CachedSacraBlackboardComponent
		? Cast<UBlackboardComponent>(CachedSacraBlackboardComponent)
		: CachedAIController->FindComponentByClass<UBlackboardComponent>();
}

void USacraEnemyContextComponent::ResolveHatredComponent()
{
	if (IsValid(CachedHatredComponent))
	{
		return;
	}

	ResolveAIController();
	if (IsValid(CachedAIController))
	{
		CachedHatredComponent = CachedAIController->FindComponentByClass<USacraEnemyHatredComponent>();
	}

	if (!IsValid(CachedHatredComponent) && GetOwner())
	{
		CachedHatredComponent = GetOwner()->FindComponentByClass<USacraEnemyHatredComponent>();
	}
}

void USacraEnemyContextComponent::ResolveWeaponComponent()
{
	if (IsValid(CachedWeaponComponent))
	{
		return;
	}

	if (const ASacraEnemy* SacraEnemy = Cast<ASacraEnemy>(GetOwner()))
	{
		CachedWeaponComponent = SacraEnemy->GetEnemyWeaponComponent();
		if (IsValid(CachedWeaponComponent))
		{
			return;
		}
	}

	CachedWeaponComponent = GetOwner() ? GetOwner()->FindComponentByClass<USacraEnemyWeaponComponent>() : nullptr;
}

void USacraEnemyContextComponent::ResolvePatrolSplineComponent()
{
	if (IsValid(CachedPatrolSplineComponent))
	{
		return;
	}

	CachedPatrolSplineComponent = GetOwner() ? GetOwner()->FindComponentByClass<UEnemyPatrolSplineComponent>() : nullptr;
}
