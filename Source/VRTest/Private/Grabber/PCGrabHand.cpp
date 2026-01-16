// Fill out your copyright notice in the Description page of Project Settings.

#include "Grabber/PCGrabHand.h"
#include "Game/BasePCPlayer.h"
#include "Camera/CameraComponent.h"
#include "Grabber/IGrabbable.h"
#include "Grabber/GrabTypes.h"
#include "Grabbee/GrabbeeObject.h"
#include "Grabbee/GrabbeeWeapon.h"
#include "GameFramework/Character.h"

UPCGrabHand::UPCGrabHand()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPCGrabHand::BeginPlay()
{
	Super::BeginPlay();
	
	// 缓存所属 PC 玩家（避免每次 Cast）
	PCPlayer = Cast<ABasePCPlayer>(GetOwner());
	if (!PCPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("UPCGrabHand::BeginPlay: Owner is not ABasePCPlayer!"));
		CachedCamera = nullptr;
		return;
	}

	CachedCamera = PCPlayer->FirstPersonCamera;
	if (!CachedCamera)
	{
		UE_LOG(LogTemp, Error, TEXT("UPCGrabHand::BeginPlay: PCPlayer->FirstPersonCamera is NULL!"));
		return;
	}

	// 默认手位：如果未在编辑器里提前配置，则使用游戏开始时手在场景里的相对位置（相对于摄像机）
	// 这样退出瞄准时可以可靠地回到开局默认位置
	const bool bHasEditorDefault = !DefaultRelativeTransform.Equals(FTransform::Identity);
	if (!bHasEditorDefault)
	{
		const FTransform CameraWorld = CachedCamera->GetComponentTransform();
		const FTransform HandWorld = GetComponentTransform();
		DefaultRelativeTransform = HandWorld.GetRelativeTransform(CameraWorld);
	}

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

void UPCGrabHand::TryGrab(bool bFromBackpack)
{
	// 先把 PC 特有的“抓箭进背包”逻辑放在最前面；一旦成功会直接 return。
	// 注意：这里也要遵守 GrabLock。
	if (bGrabLocked)
	{
		return;
	}
	if (bIsHolding)
	{
		return;
	}

	FName BoneName = NAME_None;
	AActor* TargetActor = FindTarget(bFromBackpack, BoneName);
	if (!TargetActor)
	{
		return;
	}

	if (AGrabbeeWeapon* Weapon = Cast<AGrabbeeWeapon>(TargetActor))
	{
		if (Weapon->WeaponType == EWeaponType::Arrow)
		{
			if (CachedInventory && CachedInventory->TryStoreArrow())
			{
				TargetActor->Destroy();
				return;
			}
			// 背包满则走正常抓取流程（不销毁）
		}
	}

	// Step 3: 检查目标的 GrabType
	IGrabbable* Grabbable = Cast<IGrabbable>(TargetActor);
	if (Grabbable)
	{
		EGrabType GrabType = IGrabbable::Execute_GetGrabType(TargetActor);

		// 如果是 Free 类型，先将物体移动到手部位置
		if (GrabType == EGrabType::Free)
		{
			TargetActor->SetActorLocation(GetComponentLocation());
			TargetActor->SetActorRotation(GetComponentRotation());
		}
	}

	// Step 4: 执行抓取
	GrabObject(TargetActor, BoneName);
}

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
	if (PCPlayer)
	{
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
	if (bGrabLocked)
		return;
	
	if (!bIsHolding || !HeldActor)
	{
		return;
	}

	if (!PCPlayer)
	{
		return;
	}

	// 先获取要移动的 Primitive（优先走 IGrabbable 约定），避免 Actor Root 不是碰撞体时 sweep 无效
	AActor* DroppedObject = HeldActor;
	UPrimitiveComponent* DroppedPrimitive = nullptr;
	if (DroppedObject)
	{
		if (Cast<IGrabbable>(DroppedObject))
		{
			DroppedPrimitive = IGrabbable::Execute_GetGrabPrimitive(DroppedObject);
		}

		if (!DroppedPrimitive)
		{
			DroppedPrimitive = Cast<UPrimitiveComponent>(DroppedObject->GetRootComponent());
		}
	}

	// 先释放（解除 PhysicsHandle 控制）
	ReleaseObject();

	// 没有命中目标点则正常释放即可
	if (!PCPlayer->bTraceHit || !DroppedObject || Cast<USkeletalMeshComponent>(DroppedPrimitive))
	{
		return;
	}

	const FVector TargetLocation = PCPlayer->TraceTargetLocation;

	// 如果没有可用 Primitive，就退化为 Actor sweep（效果不如 component sweep 可靠）
	if (!DroppedPrimitive)
	{
		DroppedObject->SetActorLocation(TargetLocation, true /*bSweep*/);
		return;
	}

	// 使用 Component Move + Sweep：命中则会停在碰撞外侧，避免瞬移穿透导致弹飞
	FHitResult SweepHit;
	DroppedPrimitive->MoveComponent(
		TargetLocation - DroppedPrimitive->GetComponentLocation(),
		DroppedPrimitive->GetComponentQuat(),
		true /*bSweep*/, 
		&SweepHit,
		MOVECOMP_NoFlags,
		ETeleportType::None);
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

void UPCGrabHand::UpdateHandInterp(float DeltaTime)
{
	if (!CachedCamera)
	{
		bIsInterping = false;
		return;
	}

	// 计算世界空间目标位置
	FTransform CameraTransform = CachedCamera->GetComponentTransform();
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
