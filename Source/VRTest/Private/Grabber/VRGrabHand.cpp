// Fill out your copyright notice in the Description page of Project Settings.

#include "Grabber/VRGrabHand.h"
#include "Grabber/IGrabbable.h"
#include "Grabbee/GrabbeeObject.h"
#include "Grabbee/GrabbeeWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Tools/GameUtils.h"
#include "Game/CollisionConfig.h"

UVRGrabHand::UVRGrabHand()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UVRGrabHand::BeginPlay()
{
	Super::BeginPlay();
	LastHandLocation = GetComponentLocation();
	
	if (!HandCollision)
	{
		UE_LOG(LogTemp, Error, TEXT("VRGrabHand::BeginPlay - HandCollision is NULL!"));
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
	// Step 0: 检查抓取锁（继承自父类）
	if (bGrabLocked)
	{
		return;
	}

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



void UVRGrabHand::TryRelease(bool bToBackpack)
{
	// Step 0: 检查抓取锁（继承自父类）
	if (bGrabLocked)
	{
		return;
	}

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
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const FVector Origin = GetComponentLocation();
	const FVector Forward = GetForwardVector().GetSafeNormal();

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(VRGravityGloves), false);
	QueryParams.AddIgnoredActor(GetOwner());

	const bool bHit = World->OverlapMultiByChannel(
		Overlaps,
		Origin,
		FQuat::Identity,
		TCC_GRAB,
		FCollisionShape::MakeSphere(GravityGlovesDistance),
		QueryParams
	);

	if (!bHit || Overlaps.Num() == 0)
	{
		return nullptr;
	}

	TArray<FActorWithAngle> Candidates;
	TSet<AActor*> VisitedActors;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Actor = Overlap.GetActor();
		if (!Actor || VisitedActors.Contains(Actor))
		{
			continue;
		}
		VisitedActors.Add(Actor);

		if (!Actor->GetClass()->ImplementsInterface(UGrabbable::StaticClass()))
		{
			continue;
		}

		if (!IGrabbable::Execute_CanBeGrabbedByGravityGlove(Actor))
		{
			continue;
		}

		if (!IGrabbable::Execute_CanBeGrabbedBy(Actor, this))
		{
			continue;
		}

		const FVector ToTarget = (Actor->GetActorLocation() - Origin).GetSafeNormal();
		const float Angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Forward, ToTarget)));
		if (Angle <= GravityGlovesAngle)
		{
			Candidates.Add(FActorWithAngle(Actor, Angle));
		}
	}

	Candidates.Sort([](const FActorWithAngle& A, const FActorWithAngle& B)
	{
		return A.Angle < B.Angle;
	});

	if (Candidates.Num() > 0)
	{
		return Candidates[0].Actor;
	}

	return nullptr;
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

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const FVector Origin = GetComponentLocation();
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(VRGrabOverlap), false);
	QueryParams.AddIgnoredActor(GetOwner());

	const bool bHit = World->OverlapMultiByChannel(
		Overlaps,
		Origin,
		FQuat::Identity,
		TCC_GRAB,
		FCollisionShape::MakeSphere(GrabSphereRadius),
		QueryParams
	);

	if (!bHit || Overlaps.Num() == 0)
	{
		return nullptr;
	}

	AActor* ClosestGrabbableActor = nullptr;
	float ClosestDistanceSq = FLT_MAX;
	FName ClosestBoneName = NAME_None;
	TSet<AActor*> VisitedActors;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (!HitActor || VisitedActors.Contains(HitActor))
		{
			continue;
		}
		VisitedActors.Add(HitActor);

		IGrabbable* Grabbable = Cast<IGrabbable>(HitActor);
		if (!Grabbable)
		{
			continue;
		}

		if (!IGrabbable::Execute_CanBeGrabbedBy(HitActor, this))
		{
			continue;
		}

		const UPrimitiveComponent* HitComp = Overlap.Component.Get();
		const FVector TargetLocation = HitComp ? HitComp->GetComponentLocation() : HitActor->GetActorLocation();
		const float DistanceSq = FVector::DistSquared(Origin, TargetLocation);
		if (DistanceSq < ClosestDistanceSq)
		{
			ClosestDistanceSq = DistanceSq;
			ClosestGrabbableActor = HitActor;
			ClosestBoneName = NAME_None;

			if (const USkeletalMeshComponent* SkelMesh = Cast<USkeletalMeshComponent>(HitComp))
			{
				ClosestBoneName = SkelMesh->FindClosestBone(Origin);
			}
		}
	}

	if (ClosestGrabbableActor)
	{
		OutBoneName = ClosestBoneName;
		return ClosestGrabbableActor;
	}

	return nullptr;
}

