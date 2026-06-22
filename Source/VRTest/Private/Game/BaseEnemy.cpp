// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Characters/BaseEnemy.h"
#include "AI/Component/SacraEnemyLoadoutComponent.h"
#include "AI/Component/SacraEnemyWeaponComponent.h"
#include "AI/SacraEnemySubsystem.h"
#include "Grabber/GrabTypes.h"
#include "Grabber/PlayerGrabHand.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Game/CollisionConfig.h"
#include "GameFramework/CharacterMovementComponent.h"

// ==================== IGrabbable 接口实现 ====================

ABaseEnemy::ABaseEnemy()
{
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionProfileName(CP_ENEMY_CAPSULE);
	}

	GetMesh()->SetCollisionProfileName(CP_ENEMY_MESH_ALIVE);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (USacraEnemySubsystem* EnemySubsystem = USacraEnemySubsystem::Get(this))
	{
		EnemySubsystem->RegisterEnemy(this);
	}
}

void ABaseEnemy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (USacraEnemySubsystem* EnemySubsystem = USacraEnemySubsystem::Get(this))
	{
		EnemySubsystem->UnregisterEnemy(this);
	}

	Super::EndPlay(EndPlayReason);
}

void ABaseEnemy::OnDeath_Implementation()
{
	Super::OnDeath_Implementation();

	if (USacraEnemyWeaponComponent* WeaponComponent = FindComponentByClass<USacraEnemyWeaponComponent>())
	{
		WeaponComponent->HandleOwnerDeath();
	}

	// 1. 设置 Capsule 碰撞为 NoCollision
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionProfileName(CP_NO_COLLISION);
	}

	// 2. 设置 Mesh 为 Ragdoll 并开启物理模拟
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetCollisionProfileName(CP_ENEMY_MESH_RAGDOLL);
		MeshComp->SetAllBodiesSimulatePhysics(true);
	}

	// 3. 禁用 CharacterMovement 组件
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->DisableMovement();
		Movement->StopMovementImmediately();
	}

	// 4. 删除 AliveComponent
	if (AliveComponent)
	{
		AliveComponent->DestroyComponent();
	}
}

void ABaseEnemy::TakeArrowEffect(const FEffect& Effect)
{
	if (const USacraEnemyLoadoutComponent* LoadoutComponent = FindComponentByClass<USacraEnemyLoadoutComponent>())
	{
		if (!LoadoutComponent->CanReceiveArrowDamage(Effect))
		{
			return;
		}
	}

	if (AliveComponent)
	{
		AliveComponent->DecreaseHP(Effect.Amount);
	}
}


#pragma region IGrabInterface
EGrabType ABaseEnemy::GetGrabType_Implementation() const
{
	return EGrabType::HumanBody;
}

UPrimitiveComponent* ABaseEnemy::GetGrabPrimitive_Implementation() const
{
	return GetMesh();
}

bool ABaseEnemy::CanBeGrabbedBy_Implementation(const UPlayerGrabHand* Hand) const
{
	// 只有死亡后才能被抓取
	if (!bIsDead)
	{
		return false;
	}
	
	// 不能被同一只手重复抓取
	return !ControllingHands.Contains(const_cast<UPlayerGrabHand*>(Hand));
}

bool ABaseEnemy::CanBeGrabbedByGravityGlove_Implementation() const
{
	return false;
}

bool ABaseEnemy::SupportsDualHandGrab_Implementation() const
{
	// 尸体支持双手同时抓取
	return true;
}

void ABaseEnemy::OnGrabbed_Implementation(UPlayerGrabHand* Hand)
{
	if (Hand)
	{
		ControllingHands.Add(Hand);
	}
}

void ABaseEnemy::OnReleased_Implementation(UPlayerGrabHand* Hand)
{
	if (Hand)
	{
		ControllingHands.Remove(Hand);
	}
}

void ABaseEnemy::OnGrabSelected_Implementation()
{
	// 尸体不需要选中效果，空实现
}

void ABaseEnemy::OnGrabDeselected_Implementation()
{
	// 尸体不需要取消选中效果，空实现
}

#pragma endregion IGrabInterface
