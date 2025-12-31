// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/VRGrabHand.h"
#include "Game/InventoryComponent.h"
#include "Grab/IGrabbable.h"
#include "Grabbee/GrabbeeObject.h"
#include "Grabbee/GrabbeeWeapon.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetSystemLibrary.h"

UVRGrabHand::UVRGrabHand()
{
	PrimaryComponentTick.bCanEverTick = true;

	// 默认检测 WorldDynamic 对象
	GrabObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	GrabObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody));
}

void UVRGrabHand::BeginPlay()
{
	Super::BeginPlay();
	LastHandLocation = GetComponentLocation();
}

void UVRGrabHand::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 更新手部速度
	UpdateHandVelocity(DeltaTime);

	// 更新 Gravity Gloves
	if (bEnableGravityGloves)
	{
		UpdateGravityGloves(DeltaTime);
	}
}

// ==================== 重写 ====================

void UVRGrabHand::TryGrab(bool bFromBackpack)
{
	// 如果调用者没有强制指定从背包抓取，则自动检测
	if (!bFromBackpack)
	{
		bFromBackpack = IsInBackpackArea();
	}

	Super::TryGrab(bFromBackpack);
}

AActor* UVRGrabHand::FindTarget(bool bFromBackpack, FName& OutBoneName)
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

	// 优先使用 SphereTrace 检测（可以获取骨骼名）
	AActor* NearTarget = PerformSphereTrace(OutBoneName);
	if (NearTarget)
	{
		return NearTarget;
	}

	// 如果有 Gravity Gloves 选中的目标，返回作为虚拟抓取目标
	if (bEnableGravityGloves && GravityGlovesTarget)
	{
		return GravityGlovesTarget;
	}

	return nullptr;
}

void UVRGrabHand::GrabObject(AActor* TargetActor, FName BoneName)
{
	// 注意：调用此函数前应先通过 ValidateGrab 验证
	// 判断是近距离目标还是 Gravity Gloves 目标
	// 近距离目标：直接抓取
	// Gravity Gloves 目标：虚拟抓取
	
	bool bIsNearTarget = false;
	FName TempBone;
	AActor* NearCheck = PerformSphereTrace(TempBone);
	if (NearCheck == TargetActor)
	{
		bIsNearTarget = true;
	}

	if (bIsNearTarget)
	{
		// 近距离目标：正常抓取流程（传递骨骼名）
		Super::GrabObject(TargetActor, BoneName);
	}
	else if (bEnableGravityGloves && TargetActor == GravityGlovesTarget)
	{
		// Gravity Gloves 目标：虚拟抓取
		VirtualGrab(TargetActor);
	}
	else
	{
		// 其他情况（如从背包取物）：正常抓取
		Super::GrabObject(TargetActor, BoneName);
	}
}

bool UVRGrabHand::IsInBackpackArea() const
{
	if (!BackpackCollision)
	{
		return false;
	}

	// 检查手部位置是否在背包碰撞区域内
	FVector HandLocation = GetComponentLocation();
	FVector ClosestPoint;
	float Distance = BackpackCollision->GetDistanceToCollision(HandLocation, ClosestPoint);

	return Distance <= 0.0f; // 在碰撞体内部时距离为负或零
}

void UVRGrabHand::TryRelease(bool bToBackpack)
{
	// 如果是虚拟抓取状态，进行虚拟释放（不发射）
	if (bIsVirtualGrabbing)
	{
		VirtualRelease(false);
		return;
	}

	if (!bIsHolding || !HeldActor)
	{
		return;
	}

	// 弓箭模式下，弓由Player管理，不通过手部释放
	// 这里只处理箭和其他物品

	// 箭的处理：在背包区域且未满则存入，否则掉落
	if (AGrabbeeWeapon* Weapon = Cast<AGrabbeeWeapon>(HeldActor))
	{
		if (Weapon->WeaponType == EWeaponType::Arrow)
		{
			bool bShouldStore = false;
			
			if (IsInBackpackArea())
			{
				if (UInventoryComponent* Inventory = GetInventoryComponent())
				{
					if (!Inventory->IsArrowFull())
					{
						bShouldStore = true;

						// 先保存要销毁的物体指针
						AActor* ObjectToDestroy = HeldActor;

						// 清空状态（避免悬空指针）
						HeldActor = nullptr;
						HeldGrabType = EGrabType::None;
						bIsHolding = false;
						CurrentControlName = NAME_None;

						// 释放 Attach
						ReleaseAttach();

						// 通知物体被释放（通过接口）
						if (IGrabbable* Grabbable = Cast<IGrabbable>(ObjectToDestroy))
						{
							IGrabbable::Execute_OnReleased(ObjectToDestroy, this);
							OnObjectReleased.Broadcast(ObjectToDestroy);
						}

						// 最后存入背包（会销毁Actor）
						Inventory->TryStoreArrow();
					}
				}
			}
			
			if (!bShouldStore)
			{
				// 不在背包区域或箭已满，正常掉落
				Super::TryRelease(false);
			}
			return;
		}
	}

	// 其他物品：在背包区域尝试存入，否则正常释放
	if (IsInBackpackArea())
	{
		Super::TryRelease(true); // 放入背包
	}
	else
	{
		Super::TryRelease(false); // 正常释放
	}
}

// ==================== VR 专用接口 ====================

AActor* UVRGrabHand::FindAngleClosestTarget()
{
	TArray<AActor*> OverlapActors;
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(GetOwner());

	// 大范围球形检测
	bool bHit = UKismetSystemLibrary::SphereOverlapActors(
		this,
		GetComponentLocation(),
		GravityGlovesDistance,
		GrabObjectTypes,
		nullptr,  // 不限制类型，用接口检查
		IgnoreActors,
		OverlapActors
	);

	if (!bHit || OverlapActors.Num() == 0)
	{
		return nullptr;
	}

	// 找到角度最近的目标
	AActor* ClosestTarget = nullptr;
	float SmallestAngle = GravityGlovesAngle;

	FVector HandForward = GetForwardVector();

	for (AActor* Actor : OverlapActors)
	{
		// 检查是否实现 IGrabbable 接口
		IGrabbable* Grabbable = Cast<IGrabbable>(Actor);
		if (!Grabbable)
		{
			continue;
		}
		
		if (!IGrabbable::Execute_CanBeGrabbedBy(Actor, this))
		{
			continue;
		}

		// 计算角度
		FVector ToTarget = (Actor->GetActorLocation() - GetComponentLocation()).GetSafeNormal();
		float Angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(HandForward, ToTarget)));

		if (Angle < SmallestAngle)
		{
			SmallestAngle = Angle;
			ClosestTarget = Actor;
		}
	}

	return ClosestTarget;
}

void UVRGrabHand::VirtualGrab(AActor* Target)
{
	if (!Target)
	{
		return;
	}

	// 设置虚拟抓取状态（与真实抓取相同的状态变量）
	HeldActor = Target;
	
	// 获取并缓存 GrabType
	if (IGrabbable* Grabbable = Cast<IGrabbable>(Target))
	{
		HeldGrabType = IGrabbable::Execute_GetGrabType(Target);
	}
	else
	{
		HeldGrabType = EGrabType::Free; // 默认
	}
	
	bIsHolding = true;
	bIsVirtualGrabbing = true;

	// 清除选中状态（物体从"选中"变为"虚拟抓取"）
	if (GravityGlovesTarget == Target)
	{
		GravityGlovesTarget = nullptr;
	}

	// 通知物体被选中变为抓取（但不是真正的物理抓取）
	// 不调用 OnGrabbed，因为物体还没到手
}

void UVRGrabHand::VirtualRelease(bool bLaunch)
{
	if (!bIsVirtualGrabbing || !HeldActor)
	{
		return;
	}

	AActor* ReleasedTarget = HeldActor;

	// 如果需要发射物体
	if (bLaunch)
	{
		// 只有 AGrabbeeObject 支持 LaunchTowards
		if (AGrabbeeObject* GrabbeeObj = Cast<AGrabbeeObject>(ReleasedTarget))
		{
			GrabbeeObj->LaunchTowards(GetComponentLocation(), LaunchArcParam);
		}
	}

	// 清除状态
	HeldActor = nullptr;
	HeldGrabType = EGrabType::None;
	bIsHolding = false;
	bIsVirtualGrabbing = false;
}

// ==================== 内部函数 ====================

void UVRGrabHand::UpdateGravityGloves(float DeltaTime)
{
	// 如果正在虚拟抓取，检测向后拉动手势
	if (bIsVirtualGrabbing && HeldActor)
	{
		// 检查目标是否仍然有效
		if (!IsValid(HeldActor))
		{
			VirtualRelease(false);
			return;
		}

		// 检测向后拉动手势
		if (CheckPullBackGesture())
		{
			// 发射物体并释放
			VirtualRelease(true);
		}
		return;
	}

	// 如果没有抓取物体，寻找新的目标
	if (!bIsHolding)
	{
		AActor* NewTarget = FindAngleClosestTarget();

		// 目标发生变化
		if (NewTarget != GravityGlovesTarget)
		{
			// 取消选中旧目标（通过接口）
			if (GravityGlovesTarget)
			{
				if (IGrabbable* OldGrabbable = Cast<IGrabbable>(GravityGlovesTarget))
				{
					IGrabbable::Execute_OnGrabDeselected(GravityGlovesTarget);
				}
			}

			// 选中新目标（通过接口）
			GravityGlovesTarget = NewTarget;
			if (GravityGlovesTarget)
			{
				if (IGrabbable* NewGrabbable = Cast<IGrabbable>(GravityGlovesTarget))
				{
					IGrabbable::Execute_OnGrabSelected(GravityGlovesTarget);
				}
			}
		}
	}
}

void UVRGrabHand::UpdateHandVelocity(float DeltaTime)
{
	if (DeltaTime > 0.0f)
	{
		FVector CurrentLocation = GetComponentLocation();
		HandVelocity = (CurrentLocation - LastHandLocation) / DeltaTime;
		LastHandLocation = CurrentLocation;
	}
}

bool UVRGrabHand::CheckPullBackGesture() const
{
	if (!HeldActor)
	{
		return false;
	}

	// 计算从物体指向手的方向（向后拉的方向）
	FVector ObjectToHand = (GetComponentLocation() - HeldActor->GetActorLocation()).GetSafeNormal();

	// 计算手部速度与该方向的点积
	// 点积 > 0 表示手在远离物体（向后拉）
	FVector NormalizedVelocity = HandVelocity.GetSafeNormal();
	float DotProduct = FVector::DotProduct(NormalizedVelocity, ObjectToHand);

	// 当点积超过阈值且速度足够大时，触发发射
	return DotProduct > PullBackThreshold && HandVelocity.Size() > MinPullVelocity;
}

AActor* UVRGrabHand::PerformSphereOverlap() const
{
	TArray<AActor*> OverlapActors;
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(GetOwner());

	bool bHit = UKismetSystemLibrary::SphereOverlapActors(
		this,
		GetComponentLocation(),
		GrabSphereRadius,
		GrabObjectTypes,
		nullptr,  // 不限制类型，用接口检查
		IgnoreActors,
		OverlapActors
	);

	if (bHit && OverlapActors.Num() > 0)
	{
		// 返回第一个有效的可抓取物体（实现 IGrabbable 接口）
		for (AActor* Actor : OverlapActors)
		{
			IGrabbable* Grabbable = Cast<IGrabbable>(Actor);
			if (Grabbable && IGrabbable::Execute_CanBeGrabbedBy(Actor, this))
			{
				return Actor;
			}
		}
	}

	return nullptr;
}

bool UVRGrabHand::IsInGravityGlovesAngle(AActor* Target) const
{
	if (!Target)
	{
		return false;
	}

	FVector HandForward = GetForwardVector();
	FVector ToTarget = (Target->GetActorLocation() - GetComponentLocation()).GetSafeNormal();
	float Angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(HandForward, ToTarget)));

	return Angle <= GravityGlovesAngle;
}

AActor* UVRGrabHand::PerformSphereTrace(FName& OutBoneName) const
{
	OutBoneName = NAME_None;
	
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(GetOwner());

	FHitResult HitResult;
	FVector Start = GetComponentLocation();
	FVector End = Start; // 原地球形扫描
	
	// 使用 SphereTraceSingleForObjects 来获取精确的 Hit 信息
	bool bHit = UKismetSystemLibrary::SphereTraceSingleForObjects(
		this,
		Start,
		End,
		GrabSphereRadius,
		GrabObjectTypes,
		false,  // bTraceComplex
		IgnoreActors,
		EDrawDebugTrace::None,
		HitResult,
		true    // bIgnoreSelf
	);

	if (bHit && HitResult.GetActor())
	{
		AActor* HitActor = HitResult.GetActor();
		IGrabbable* Grabbable = Cast<IGrabbable>(HitActor);
		if (Grabbable && IGrabbable::Execute_CanBeGrabbedBy(HitActor, this))
		{
			// 从 HitResult 获取骨骼名
			OutBoneName = HitResult.BoneName;
			return HitActor;
		}
	}

	// 回退到 Overlap 检测（不返回骨骼名，用于非骨骼网格体）
	return PerformSphereOverlap();
}
