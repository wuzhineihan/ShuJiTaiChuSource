// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BasePlayer.h"
#include "Grabber/PlayerGrabHand.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Grabbee/GrabbeeWeapon.h"
#include "Grabbee/Bow.h"

ABasePlayer::ABasePlayer()
{
	FallDamageComponent = CreateDefaultSubobject<UFallDamageComponent>(TEXT("FallDamageComponent"));
	AutoRecoverComponent = CreateDefaultSubobject<UAutoRecoverComponent>(TEXT("AutoRecoverComponent"));
	
	// 为左右手分别创建 PhysicsHandleComponent
	LeftPhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("LeftPhysicsHandle"));
	RightPhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("RightPhysicsHandle"));
}

void ABasePlayer::BeginPlay()
{
	Super::BeginPlay();

	// 统一设置手部的 PhysicsHandle 和 Inventory
	if (LeftHand)
	{
		LeftHand->SetPhysicsHandle(LeftPhysicsHandle);
		LeftHand->SetInventory(InventoryComponent);
	}
	if (RightHand)
	{
		RightHand->SetPhysicsHandle(RightPhysicsHandle);
		RightHand->SetInventory(InventoryComponent);
	}
}

// ==================== 弓接口 ====================

void ABasePlayer::OnBowFirstPickedUp()
{
	if (bHasBow)
	{
		return; // 已经有弓了
	}

	bHasBow = true;
	
	// 首次拾取弓时自动进入弓箭模式
	SetBowArmed(true);
}

void ABasePlayer::SetBowArmed(bool bArmed)
{
	if (!bHasBow || bIsBowArmed == bArmed)
	{
		return;
	}

	bIsBowArmed = bArmed;

	if (bArmed)
	{
		// 进入弓箭模式：生成弓
		CurrentBow = SpawnBow();
	}
	else
	{
		// 退出弓箭模式：销毁弓
		DestroyBow();
	}

	OnBowArmedChanged.Broadcast(bIsBowArmed);
}

ABow* ABasePlayer::SpawnBow()
{
	if (!BowClass)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;

	return GetWorld()->SpawnActor<ABow>(BowClass, GetActorTransform(), SpawnParams);
}

void ABasePlayer::DestroyBow()
{
	if (CurrentBow)
	{
		CurrentBow->Destroy();
		CurrentBow = nullptr;
	}
}
