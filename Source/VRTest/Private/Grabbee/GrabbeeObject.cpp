// Fill out your copyright notice in the Description page of Project Settings.

#include "Grabbee/GrabbeeObject.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"

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
}

void AGrabbeeObject::OnReleased_Implementation(UPlayerGrabHand* Hand)
{
	bIsHeld = false;
	HoldingHand = nullptr;
}

void AGrabbeeObject::CustomGrab_Implementation(UPlayerGrabHand* Hand)
{
	// 默认空实现，子类重写
}

void AGrabbeeObject::CustomRelease_Implementation(UPlayerGrabHand* Hand)
{
	// 默认空实现，子类重写
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
