// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BasePlayer.h"

#include "Camera/CameraComponent.h"
#include "Grabber/PlayerGrabHand.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Components/CapsuleComponent.h"
#include "Grabbee/Bow.h"
#include "Game/GameSettings.h"
#include "Skill/PlayerSkillComponent.h"
#include "Materials/MaterialInterface.h"

ABasePlayer::ABasePlayer()
{
	FallDamageComponent = CreateDefaultSubobject<UFallDamageComponent>(TEXT("FallDamageComponent"));
	AutoRecoverComponent = CreateDefaultSubobject<UAutoRecoverComponent>(TEXT("AutoRecoverComponent"));
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	PlayerSkillComponent = CreateDefaultSubobject<UPlayerSkillComponent>(TEXT("PlayerSkillComponent"));

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionProfileName(FName("Profile_PlayerCapsule"));
	}

	
	// 为左右手分别创建 PhysicsHandleComponent
	LeftPhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("LeftPhysicsHandle"));
	RightPhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("RightPhysicsHandle"));
}

void ABasePlayer::BeginPlay()
{
	Super::BeginPlay();

	// 给玩家相机添加后处理材质（从 GameSettings 配置）
	if (PlayerCamera)
	{
		if (UGameSettings* Settings = UGameSettings::Get())
		{
			if (UMaterialInterface* PPMat = Settings->GetPlayerCameraPostProcessMaterial())
			{
				// 避免重复添加同一个 Blendable
				bool bAlreadyAdded = false;
				for (const FWeightedBlendable& WB : PlayerCamera->PostProcessSettings.WeightedBlendables.Array)
				{
					if (WB.Object == PPMat)
					{
						bAlreadyAdded = true;
						break;
					}
				}

				if (!bAlreadyAdded)
				{
					PlayerCamera->PostProcessSettings.AddBlendable(PPMat, 1.0f);
				}
			}
		}
	}

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

	PlayerController = Cast<APlayerController>(GetController());
}

// ==================== 弓 ====================

bool ABasePlayer::CheckBowFirstPickedUp()
{
	if (bHasBow)
	{
		return false; // 已经有弓了
	}
	
	bHasBow = true;
	
	// 首次拾取弓时自动进入弓箭模式
	SetBowArmed(true);
	return true;
}

void ABasePlayer::SetBowArmed(bool bArmed)
{
	if (!bHasBow || bIsBowArmed == bArmed)
	{
		return;
	}

	// 退出弓箭模式时先释放左手
	if (bIsBowArmed && !bArmed)
	{
		if (LeftHand && LeftHand->bIsHolding && LeftHand->HeldActor == CurrentBow)
		{
			LeftHand->ReleaseObject();
		}
	}
	
	if (bArmed)
	{
		//释放双手
		if (LeftHand && LeftHand->bIsHolding)
		{
			LeftHand->ReleaseObject();
		}
		if (RightHand && RightHand->bIsHolding)
		{
			RightHand->ReleaseObject();
		}
		
		// 进入弓箭模式：生成弓
		CurrentBow = SpawnBow();
		
		// 左手自动抓取弓
		if (CurrentBow && LeftHand)
		{
			LeftHand->GrabObject(CurrentBow);
		}
	}
	else
	{
		// 退出弓箭模式：销毁弓
		DestroyBow();
	}
	
	bIsBowArmed = bArmed;
}

bool ABasePlayer::GetBowArmed() const
{
	return bIsBowArmed;
}

void ABasePlayer::PlaySimpleForceFeedback(EControllerHand Hand)
{
	// 子类实现
}

ABow* ABasePlayer::SpawnBow()
{
	// 从 GameSettings 获取弓的蓝图类
	TSubclassOf<ABow> BowClass = UGameSettings::Get()->GetBowClass();
	if (!BowClass)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnBow: BowClass is not set in Project Settings -> Game -> Game Settings!"));
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
