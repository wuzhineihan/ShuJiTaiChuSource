// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BaseEnemy.h"
#include "Grabber/GrabTypes.h"
#include "Grabber/PlayerGrabHand.h"
#include "Components/SkeletalMeshComponent.h"

// ==================== IGrabbable 接口实现 ====================

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
