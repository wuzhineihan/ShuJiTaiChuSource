// Fill out your copyright notice in the Description page of Project Settings.

#include "Grabbee/GrabbeeObject.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Game/PlayerGrabHand.h"

AGrabbeeObject::AGrabbeeObject()
{
	PrimaryActorTick.bCanEverTick = false;

	// 创建静态网格体组件作为根组件
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// 默认启用物理模拟
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void AGrabbeeObject::BeginPlay()
{
	Super::BeginPlay();
}

// ==================== IGrabbable 接口实现 ====================

EGrabType AGrabbeeObject::GetGrabType_Implementation() const
{
	return GrabType;
}

UPrimitiveComponent* AGrabbeeObject::GetGrabPrimitive_Implementation() const
{
	return MeshComponent;
}

bool AGrabbeeObject::CanBeGrabbedBy_Implementation(const UPlayerGrabHand* Hand) const
{
	// 检查手是否有效
	if (!Hand)
	{
		return false;
	}

	// 基本检查
	if (!bCanGrab)
	{
		return false;
	}

	// 如果物体支持双手抓取（如 HumanBody）
	if (bSupportsDualHandGrab)
	{
		// 允许抓取，即使已被其他手抓取（但不能是同一只手）
		return !ControllingHands.Contains(const_cast<UPlayerGrabHand*>(Hand));
	}

	// 普通物体：未被抓取时才可抓取
	return !bIsHeld;
}

bool AGrabbeeObject::SupportsDualHandGrab_Implementation() const
{
	return bSupportsDualHandGrab;
}

void AGrabbeeObject::OnGrabbed_Implementation(UPlayerGrabHand* Hand)
{
	bIsHeld = true;
	
	// 如果支持双手抓取，添加到控制列表
	if (bSupportsDualHandGrab)
	{
		ControllingHands.Add(Hand);
		// HoldingHand 指向第一只抓取的手
		if (!HoldingHand)
		{
			HoldingHand = Hand;
		}
	}
	else
	{
		HoldingHand = Hand;
	}
	
	// 被抓取时取消选中状态
	if (bIsSelected)
	{
		bIsSelected = false;
	}
}

void AGrabbeeObject::OnReleased_Implementation(UPlayerGrabHand* Hand)
{
	// 如果支持双手抓取，从控制列表移除
	if (bSupportsDualHandGrab)
	{
		ControllingHands.Remove(Hand);
		
		// 如果还有手在控制，更新 HoldingHand
		if (ControllingHands.Num() > 0)
		{
			// 如果当前释放的是 HoldingHand，切换到另一只手
			if (HoldingHand == Hand)
			{
				for (UPlayerGrabHand* OtherHand : ControllingHands)
				{
					HoldingHand = OtherHand;
					break;
				}
			}
			// 物体仍然被抓取
			return;
		}
	}
	
	// 完全释放
	bIsHeld = false;
	HoldingHand = nullptr;
}

void AGrabbeeObject::OnGrabSelected_Implementation()
{
	bIsSelected = true;
	// 子类可重写添加高亮、音效等
}

void AGrabbeeObject::OnGrabDeselected_Implementation()
{
	bIsSelected = false;
	// 子类可重写移除高亮等
}

// ==================== 自有函数 ====================

bool AGrabbeeObject::LaunchTowards(const FVector& TargetLocation, float ArcParam)
{
	UPrimitiveComponent* Primitive = GetGrabPrimitive();
	if (!Primitive)
	{
		return false;
	}

	FVector StartLocation = GetActorLocation();
	FVector OutLaunchVelocity;

	// 使用 SuggestProjectileVelocity_CustomArc 计算发射速度
	bool bSuccess = UGameplayStatics::SuggestProjectileVelocity_CustomArc(
		this,
		OutLaunchVelocity,
		StartLocation,
		TargetLocation,
		0.0f,  // OverrideGravityZ - 使用世界重力
		ArcParam  // ArcParam - 抛物线弧度
	);

	if (bSuccess)
	{
		// 使用 VelChange = true，直接设置速度而非施加力
		Primitive->AddImpulse(OutLaunchVelocity, NAME_None, true);
		return true;
	}

	return false;
}

void AGrabbeeObject::SetSimulatePhysics(bool bSimulate)
{
	if (MeshComponent)
	{
		MeshComponent->SetSimulatePhysics(bSimulate);
	}
}

void AGrabbeeObject::ForceRelease()
{
	if (HoldingHand)
	{
		// 通知手释放此物体
		// 由于循环依赖，这里只设置状态，实际释放由 Hand 处理
		OnReleased_Implementation(HoldingHand);
	}
}
