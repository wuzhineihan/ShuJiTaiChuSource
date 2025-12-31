// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/InventoryComponent.h"
#include "Grabbee/GrabbeeObject.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

// ==================== 箭操作 ====================

bool UInventoryComponent::TryStoreArrow()
{
	if (ArrowCount >= MaxArrowCount)
	{
		// 箭已满
		return false;
	}

	ArrowCount++;
	OnArrowCountChanged.Broadcast(ArrowCount, MaxArrowCount);
	return true;
}

AGrabbeeObject* UInventoryComponent::TryRetrieveArrow(const FTransform& SpawnTransform)
{
	if (ArrowCount <= 0 || !ArrowClass)
	{
		return nullptr;
	}

	ArrowCount--;
	OnArrowCountChanged.Broadcast(ArrowCount, MaxArrowCount);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = Cast<APawn>(GetOwner());

	return GetWorld()->SpawnActor<AGrabbeeObject>(ArrowClass, SpawnTransform, SpawnParams);
}

bool UInventoryComponent::ConsumeArrow()
{
	if (ArrowCount <= 0)
	{
		return false;
	}

	ArrowCount--;
	OnArrowCountChanged.Broadcast(ArrowCount, MaxArrowCount);
	return true;
}

bool UInventoryComponent::AddArrow()
{
	if (ArrowCount >= MaxArrowCount)
	{
		return false;
	}

	ArrowCount++;
	OnArrowCountChanged.Broadcast(ArrowCount, MaxArrowCount);
	return true;
}

// ==================== 直接设置 ====================

void UInventoryComponent::SetArrowCount(int32 NewCount)
{
	NewCount = FMath::Clamp(NewCount, 0, MaxArrowCount);
	if (ArrowCount != NewCount)
	{
		ArrowCount = NewCount;
		OnArrowCountChanged.Broadcast(ArrowCount, MaxArrowCount);
	}
}

void UInventoryComponent::AddArrows(int32 Amount)
{
	SetArrowCount(ArrowCount + Amount);
}
