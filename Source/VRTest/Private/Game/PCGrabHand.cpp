// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/PCGrabHand.h"
#include "Game/BasePCPlayer.h"
#include "Camera/CameraComponent.h"
#include "Grab/IGrabbable.h"
#include "Grabbee/GrabbeeObject.h"
#include "GameFramework/Character.h"

UPCGrabHand::UPCGrabHand()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPCGrabHand::BeginPlay()
{
	Super::BeginPlay();
	
	// 初始化目标变换为默认位置
	TargetRelativeTransform = DefaultRelativeTransform;
}

void UPCGrabHand::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 更新手部插值位置
	if (bIsInterping)
	{
		UpdateHandInterp(DeltaTime);
	}

	// 注意：目标检测现在由 BasePCPlayer 统一处理，这里不需要重复检测
}

// ==================== 重写 ====================

AActor* UPCGrabHand::FindTarget(bool bFromBackpack, FName& OutBoneName)
{
	OutBoneName = NAME_None;
	
	// 优先从背包取物
	if (bFromBackpack)
	{
		FName TempBone;
		AActor* BackpackTarget = Super::FindTarget(bFromBackpack, TempBone);
		if (BackpackTarget)
		{
			return BackpackTarget;
		}
	}

	// 使用 Player 的检测结果（避免重复检测）
	if (ABasePCPlayer* PCPlayer = GetOwnerPCPlayer())
	{
		// 获取射线检测的骨骼名
		OutBoneName = PCPlayer->TargetedBoneName;
		return PCPlayer->TargetedObject;
	}

	return nullptr;
}

// ==================== PC 专用接口 ====================

void UPCGrabHand::TryGrabOrRelease()
{
	if (bIsHolding && HeldActor)
	{
		// 手里有东西 → 丢弃到射线目标位置
		DropToRaycastTarget();
	}
	else
	{
		// 手里没东西 → 尝试拾取
		TryGrab(false);
	}
}

void UPCGrabHand::DropToRaycastTarget()
{
	if (!bIsHolding || !HeldActor)
	{
		return;
	}

	FHitResult Hit;
	if (PerformLineTrace(Hit, MaxDropDistance))
	{
		// 释放物体
		AActor* DroppedObject = HeldActor;
		ReleaseObject();

		// 将物体瞬移到射线碰撞点
		if (DroppedObject)
		{
			DroppedObject->SetActorLocation(Hit.Location);
		}
	}
	else
	{
		// 没有找到碰撞点，正常释放
		ReleaseObject();
	}
}

void UPCGrabHand::InterpToTransform(const FTransform& RelativeTransform)
{
	TargetRelativeTransform = RelativeTransform;
	bIsInterping = true;
}

void UPCGrabHand::InterpToDefaultTransform()
{
	InterpToTransform(DefaultRelativeTransform);
}

// ==================== 内部函数 ====================

UCameraComponent* UPCGrabHand::GetOwnerCamera() const
{
	if (AActor* Owner = GetOwner())
	{
		return Owner->FindComponentByClass<UCameraComponent>();
	}
	return nullptr;
}

ABasePCPlayer* UPCGrabHand::GetOwnerPCPlayer() const
{
	return Cast<ABasePCPlayer>(GetOwner());
}

bool UPCGrabHand::PerformLineTrace(FHitResult& OutHit, float MaxDistance) const
{
	UCameraComponent* Camera = GetOwnerCamera();
	if (!Camera)
	{
		return false;
	}

	FVector Start = Camera->GetComponentLocation();
	FVector End = Start + Camera->GetForwardVector() * MaxDistance;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());

	return GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, GrabTraceChannel, QueryParams);
}

void UPCGrabHand::UpdateHandInterp(float DeltaTime)
{
	UCameraComponent* Camera = GetOwnerCamera();
	if (!Camera)
	{
		bIsInterping = false;
		return;
	}

	// 计算世界空间目标位置
	FTransform CameraTransform = Camera->GetComponentTransform();
	FTransform WorldTargetTransform = TargetRelativeTransform * CameraTransform;

	// 当前位置
	FTransform CurrentTransform = GetComponentTransform();

	// 插值
	FVector NewLocation = FMath::VInterpTo(CurrentTransform.GetLocation(), WorldTargetTransform.GetLocation(), DeltaTime, HandInterpSpeed);
	FQuat NewRotation = FQuat::Slerp(CurrentTransform.GetRotation(), WorldTargetTransform.GetRotation(), DeltaTime * HandInterpSpeed);

	SetWorldLocationAndRotation(NewLocation, NewRotation);

	// 检查是否到达目标
	float DistSq = FVector::DistSquared(NewLocation, WorldTargetTransform.GetLocation());
	if (DistSq < 1.0f) // 阈值
	{
		bIsInterping = false;
	}
}
