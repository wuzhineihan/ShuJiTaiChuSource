// Fill out your copyright notice in the Description page of Project Settings.

#include "Grabbee/GrabbeeObject.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Grabber/PlayerGrabHand.h"
#include "Game/BaseCharacter.h"

AGrabbeeObject::AGrabbeeObject()
{
	PrimaryActorTick.bCanEverTick = false;

	// 创建静态网格体组件作为根组件
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// 默认启用物理模拟
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetCollisionProfileName(FName("IgnoreOnlyPawn"));

	MeshComponent->SetRenderCustomDepth(true);
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

	// 普通物体（不支持双手抓取）
	if (bIsHeld)
	{
		// 如果被另一只手持有，允许抓取（会触发换手）
		// HoldingHand 存在且有 OtherHand，并且 OtherHand 就是当前尝试抓取的手
		if (HoldingHand && HoldingHand->OtherHand == Hand)
		{
			return true;
		}
		// 被同一只手或其他情况持有，不允许
		return false;
	}

	// 未被抓取时可以抓取
	return true;
}

bool AGrabbeeObject::CanBeGrabbedByGravityGlove_Implementation() const
{
	return true;
}

bool AGrabbeeObject::SupportsDualHandGrab_Implementation() const
{
	return bSupportsDualHandGrab;
}

void AGrabbeeObject::OnGrabbed_Implementation(UPlayerGrabHand* Hand)
{
	bIsHeld = true;
	
	// 设置 OwningCharacter（控制此物体的角色）
	if (Hand)
	{
		if (ABaseCharacter* Character = Cast<ABaseCharacter>(Hand->GetOwner()))
		{
			OwningCharacter = Character;
		}
	}
	
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
	MeshComponent->SetCustomDepthStencilValue(4); // 使用 setter 方法确保渲染状态更新
}

void AGrabbeeObject::OnGrabDeselected_Implementation()
{
	bIsSelected = false;
	MeshComponent->SetCustomDepthStencilValue(0); // 使用 setter 方法确保渲染状态更新
}

// ==================== 自有函数 ====================

bool AGrabbeeObject::LaunchTowards(const FVector& TargetLocation, float ArcParam)
{
	UPrimitiveComponent* Primitive = IGrabbable::Execute_GetGrabPrimitive(this);
	if (!Primitive)
	{
		return false;
	}

	// 先将物体速度置零，避免残留速度影响发射轨迹
	Primitive->SetPhysicsLinearVelocity(FVector::ZeroVector);
	Primitive->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

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
