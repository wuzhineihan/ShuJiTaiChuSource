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
	GetMesh()->SetCollisionProfileName("CharacterMesh");
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
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

void ABaseEnemy::OnDeath_Implementation()
{
	Super::OnDeath_Implementation();

	// 1. 设置 Capsule 碰撞为 NoCollision
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionProfileName(FName("NoCollision"));
	}

	// 2. 设置 Mesh 为 Ragdoll 并开启物理模拟
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetCollisionProfileName(FName("IgnoreOnlyPawn"));
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
