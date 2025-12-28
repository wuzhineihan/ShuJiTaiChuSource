// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

// ==================== 弓操作 ====================

bool UInventoryComponent::TryStoreBow()
{
	if (bBowInBackpack)
	{
		// 背包里已经有弓了
		return false;
	}

	bBowInBackpack = true;
	OnBowStateChanged.Broadcast(true);
	return true;
}

AActor* UInventoryComponent::TryRetrieveBow(const FTransform& SpawnTransform)
{
	if (!bBowInBackpack || !BowClass)
	{
		return nullptr;
	}

	bBowInBackpack = false;
	OnBowStateChanged.Broadcast(false);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = Cast<APawn>(GetOwner());

	return GetWorld()->SpawnActor<AActor>(BowClass, SpawnTransform, SpawnParams);
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

AActor* UInventoryComponent::TryRetrieveArrow(const FTransform& SpawnTransform)
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

	return GetWorld()->SpawnActor<AActor>(ArrowClass, SpawnTransform, SpawnParams);
}

// ==================== 通用操作 ====================

bool UInventoryComponent::HasItemToRetrieve(bool& OutIsBow) const
{
	if (bBowInBackpack)
	{
		OutIsBow = true;
		return true;
	}
	if (ArrowCount > 0)
	{
		OutIsBow = false;
		return true;
	}
	OutIsBow = false;
	return false;
}

AActor* UInventoryComponent::RetrieveItem(const FTransform& SpawnTransform)
{
	// 优先取弓
	if (bBowInBackpack)
	{
		return TryRetrieveBow(SpawnTransform);
	}
	// 然后取箭
	if (ArrowCount > 0)
	{
		return TryRetrieveArrow(SpawnTransform);
	}
	return nullptr;
}

bool UInventoryComponent::TryStoreItem(AActor* Item)
{
	if (!Item)
	{
		return false;
	}

	// 检查是否是弓
	if (BowClass && Item->IsA(BowClass))
	{
		if (TryStoreBow())
		{
			Item->Destroy();
			return true;
		}
		return false;
	}

	// 检查是否是箭
	if (ArrowClass && Item->IsA(ArrowClass))
	{
		if (TryStoreArrow())
		{
			Item->Destroy();
			return true;
		}
		return false;
	}

	return false;
}

// ==================== 直接设置 ====================

void UInventoryComponent::SetBowState(bool bInBackpack)
{
	if (bBowInBackpack != bInBackpack)
	{
		bBowInBackpack = bInBackpack;
		OnBowStateChanged.Broadcast(bBowInBackpack);
	}
}

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
