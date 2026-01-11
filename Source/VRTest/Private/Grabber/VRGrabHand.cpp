// Fill out your copyright notice in the Description page of Project Settings.

#include "Grabber/VRGrabHand.h"
#include "Game/InventoryComponent.h"
#include "Grabber/IGrabbable.h"
#include "Grabbee/GrabbeeObject.h"
#include "Grabbee/GrabbeeWeapon.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetSystemLibrary.h"

UVRGrabHand::UVRGrabHand()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UVRGrabHand::BeginPlay()
{
	Super::BeginPlay();
	LastHandLocation = GetComponentLocation();

	// 绑定 HandCollision 的 overlap 事件用于背包检测
	if (HandCollision)
	{
		HandCollision->OnComponentBeginOverlap.AddDynamic(this, &UVRGrabHand::OnHandCollisionBeginOverlap);
		HandCollision->OnComponentEndOverlap.AddDynamic(this, &UVRGrabHand::OnHandCollisionEndOverlap);
	}
}

void UVRGrabHand::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 更新 Gravity Gloves（手部速度追踪在虚拟抓取时才进行）
	UpdateGravityGloves(DeltaTime);
}

// ==================== 重写 ====================

void UVRGrabHand::TryGrab(bool bFromBackpack)
{
	// 如果调用者没有强制指定从背包抓取，则自动检测
	if (!bFromBackpack)
	{
		bFromBackpack = bIsInBackpackArea;
	}

	// 查找目标
	FName BoneName;
	AActor* Target = FindTarget(bFromBackpack, BoneName);
	
	if (!Target)
	{
		return;
	}

	// VR特有：远距离 Gravity Gloves 目标 → 虚拟抓取
	if (Target == GravityGlovesTarget)
	{
		VirtualGrab(Target);
		return;
	}
	
	// 近距离目标：正常抓取
	GrabObject(Target, BoneName);
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
	if (GravityGlovesTarget)
	{
		return GravityGlovesTarget;
	}

	return nullptr;
}

void UVRGrabHand::OnHandCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 检查是否是背包碰撞区域（通过 Tag 检测）
	if (OtherComp && OtherComp->ComponentHasTag(FName("player_backpack")))
	{
		bIsInBackpackArea = true;
	}
}

void UVRGrabHand::OnHandCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// 检查是否离开背包碰撞区域
	if (OtherComp && OtherComp->ComponentHasTag(FName("player_backpack")))
	{
		bIsInBackpackArea = false;
	}
}

void UVRGrabHand::TryRelease(bool bToBackpack)
{

	// VR特有：处理虚拟抓取
	if (bIsVirtualGrabbing)
	{
		VirtualRelease(false);
		return;
	}

	// VR特有：检测箭在背包区域
	if (AGrabbeeWeapon* Weapon = Cast<AGrabbeeWeapon>(HeldActor))
	{
		if (Weapon->WeaponType == EWeaponType::Arrow && bIsInBackpackArea)
		{
			bToBackpack = true;
		}
	}

	// 统一调用父类处理
	Super::TryRelease(bToBackpack);
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
		
		// 使用 CanBeGrabbedByGravityGlove 检查是否可以被重力手套选中
		if (!IGrabbable::Execute_CanBeGrabbedByGravityGlove(Actor) || !IGrabbable::Execute_CanBeGrabbedBy(Actor,this))
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

	// 重置手部速度追踪，避免残留数据触发误判
	LastHandLocation = GetComponentLocation();
	HandVelocity = FVector::ZeroVector;

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

	// 通知物体取消选中（虚拟抓取结束）
	if (IGrabbable* Grabbable = Cast<IGrabbable>(ReleasedTarget))
	{
		IGrabbable::Execute_OnGrabDeselected(ReleasedTarget);
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

		// 只在虚拟抓取时追踪手部速度（用于后拉手势检测）
		UpdateHandVelocity(DeltaTime);

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

	TArray<FHitResult> HitResults;
	FVector Start = GetComponentLocation();
	FVector End = Start; // 原地球形扫描
	
	// 使用 SphereTraceMultiForObjects 来获取所有碰撞物体
	bool bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(
		this,
		Start,
		End,
		GrabSphereRadius,
		GrabObjectTypes,
		false,  // bTraceComplex
		IgnoreActors,
		EDrawDebugTrace::ForDuration,
		HitResults,
		true    // bIgnoreSelf
	);
	
	if (!bHit || HitResults.Num() == 0)
	{
		return nullptr;
	}
	
	// 遍历所有碰撞结果，找到最近的实现了 IGrabbable 接口的物体
	AActor* ClosestGrabbableActor = nullptr;
	float ClosestDistance = FLT_MAX;
	FName ClosestBoneName = NAME_None;
	
	for (const FHitResult& HitResult : HitResults)
	{
		AActor* HitActor = HitResult.GetActor();
		if (!HitActor)
		{
			continue;
		}
		
		// 检查是否实现 IGrabbable 接口
		IGrabbable* Grabbable = Cast<IGrabbable>(HitActor);
		if (!Grabbable)
		{
			continue;
		}
		
		// 检查是否可以被抓取
		if (!IGrabbable::Execute_CanBeGrabbedBy(HitActor, this))
		{
			continue;
		}
		
		// 计算距离
		float Distance = HitResult.Distance;
		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestGrabbableActor = HitActor;
			ClosestBoneName = HitResult.BoneName;
		}
	}
	
	if (ClosestGrabbableActor)
	{
		OutBoneName = ClosestBoneName;
		return ClosestGrabbableActor;
	}
	
	return nullptr;
}

