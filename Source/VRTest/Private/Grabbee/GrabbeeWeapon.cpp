// Fill out your copyright notice in the Description page of Project Settings.

#include "Grabbee/GrabbeeWeapon.h"

AGrabbeeWeapon::AGrabbeeWeapon()
{
	// 武器默认使用 WeaponSnap 类型
	GrabType = EGrabType::WeaponSnap;
	
	// 武器不需要物理模拟（Attach 到手上）
	if (MeshComponent)
	{
		MeshComponent->SetSimulatePhysics(false);
	}
}

void AGrabbeeWeapon::OnGrabbed_Implementation(UPlayerGrabHand* Hand)
{
	Super::OnGrabbed_Implementation(Hand);
	
	// WeaponSnap 类型会由 Hand 处理 Attach 逻辑
}

void AGrabbeeWeapon::OnReleased_Implementation(UPlayerGrabHand* Hand)
{
	Super::OnReleased_Implementation(Hand);
	
	// 武器释放时由 Hand 处理 Detach 逻辑
}
