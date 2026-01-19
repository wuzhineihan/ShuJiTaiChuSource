// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BaseEnemy.h"

#include "Grabber/GrabTypes.h"
#include "Grabber/PlayerGrabHand.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// ==================== IGrabbable 接口实现 ====================

ABaseEnemy::ABaseEnemy()
{
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionProfileName(FName("Profile_EnemyCapsule"));
	}
	GetMesh()->SetCollisionProfileName(FName("Profile_EnemyMesh_Alive"));
	
}

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
	Execute_ExitStasis(this);
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

void ABaseEnemy::EnterStasis_Implementation(double TimeToStasis)
{
	// 死亡敌人不允许被定身（体验差，已禁用）
	if (bIsDead)
	{
		return;
	}

	bIsInStasis = true;
	// TODO: 活着的敌人被定身逻辑
}

void ABaseEnemy::ExitStasis_Implementation()
{
	if (bIsDead)
	{
		EnterRagdollMode();
		return;
	}
	
	bIsInStasis = false;
	// TODO: 活着的敌人退出定身逻辑
}

bool ABaseEnemy::IsInStasis_Implementation()
{
	return bIsInStasis;
}

bool ABaseEnemy::CanEnterStasis_Implementation()
{
	// 彻底禁止对尸体施加定身
	if (bIsDead)
	{
		return false;
	}

	// TODO: 活着的敌人是否允许进入定身（例如抗性/状态/AI等）
	return true;
}

void ABaseEnemy::OnDeath_Implementation()
{
	Super::OnDeath_Implementation();
	
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionProfileName(FName("NoCollision"));
	}

	if (!bIsInStasis)
		EnterRagdollMode();
	
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->DisableMovement();
		Movement->StopMovementImmediately();
	}
	
	if (AliveComponent)
	{
		AliveComponent->DestroyComponent();
	}
}

void ABaseEnemy::EnterRagdollMode()
{
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetCollisionProfileName(FName("Profile_EnemyMesh_Ragdoll"));
		MeshComp->SetAllBodiesSimulatePhysics(true);
		MeshComp->SetGenerateOverlapEvents(true);
	}
}
