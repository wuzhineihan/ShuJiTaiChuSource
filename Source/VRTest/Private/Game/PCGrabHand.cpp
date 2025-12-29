// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/PCGrabHand.h"
#include "Camera/CameraComponent.h"
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
}

// ==================== 重写 ====================

AGrabbeeObject* UPCGrabHand::FindTarget_Implementation()
{
	FHitResult Hit;
	if (PerformLineTrace(Hit, MaxGrabDistance))
	{
		return Cast<AGrabbeeObject>(Hit.GetActor());
	}
	return nullptr;
}

// ==================== PC 专用接口 ====================

void UPCGrabHand::TryGrabOrRelease()
{
	if (bIsHolding && HeldObject)
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
	if (!bIsHolding || !HeldObject)
	{
		return;
	}

	FHitResult Hit;
	if (PerformLineTrace(Hit, MaxDropDistance))
	{
		// 释放物体
		AGrabbeeObject* DroppedObject = HeldObject;
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
