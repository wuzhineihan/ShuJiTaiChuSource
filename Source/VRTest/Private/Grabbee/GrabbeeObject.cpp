// Fill out your copyright notice in the Description page of Project Settings.

#include "Grabbee/GrabbeeObject.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"

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

bool AGrabbeeObject::CanBeGrabbedBy_Implementation(UPlayerGrabHand* Hand) const
{
	return bCanGrab && !bIsHeld;
}

void AGrabbeeObject::OnGrabbed_Implementation(UPlayerGrabHand* Hand)
{
	bIsHeld = true;
	HoldingHand = Hand;
	
	// 被抓取时取消选中状态
	if (bIsSelected)
	{
		bIsSelected = false;
	}
}

void AGrabbeeObject::OnReleased_Implementation(UPlayerGrabHand* Hand)
{
	bIsHeld = false;
	HoldingHand = nullptr;
}

void AGrabbeeObject::OnSelected_Implementation()
{
	bIsSelected = true;
	// 子类可重写添加高亮、音效等
}

void AGrabbeeObject::OnDeselected_Implementation()
{
	bIsSelected = false;
	// 子类可重写移除高亮等
}

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

UPrimitiveComponent* AGrabbeeObject::GetGrabPrimitive_Implementation() const
{
	return MeshComponent;
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
		OnReleased(HoldingHand);
	}
}
