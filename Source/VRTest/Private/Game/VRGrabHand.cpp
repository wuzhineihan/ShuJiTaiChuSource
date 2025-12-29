// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/VRGrabHand.h"
#include "Game/InventoryComponent.h"
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

AGrabbeeObject* UVRGrabHand::FindTarget_Implementation()
{
	// 优先检测近距离球形重叠
	return PerformSphereOverlap();
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

void UVRGrabHand::TryGrab(bool bFromBackpack)
{
	// 如果已经持有物体，不再抓取
	if (bIsHolding)
	{
		return;
	}

	// 优先处理近距离抓取
	AGrabbeeObject* NearTarget = FindTarget();
	if (NearTarget)
	{
		// 正常抓取流程
		Super::TryGrab(bFromBackpack);
		return;
	}

	// 如果有 Gravity Gloves 选中的目标，进行虚拟抓取
	if (bEnableGravityGloves && GravityGlovesTarget)
	{
		VirtualGrab(GravityGlovesTarget);
	}
	else if (bFromBackpack)
	{
		// 从背包取物
		Super::TryGrab(true);
	}
}

void UVRGrabHand::TryRelease(bool bToBackpack)
{
	// 如果是虚拟抓取状态，进行虚拟释放（不发射）
	if (bIsVirtualGrabbing)
	{
		VirtualRelease(false);
		return;
	}

	if (!bIsHolding || !HeldObject)
	{
		return;
	}

	// 弓箭模式下，弓由Player管理，不通过手部释放
	// 这里只处理箭和其他物品

	// 箭的处理：在背包区域且未满则存入，否则掉落
	if (AGrabbeeWeapon* Weapon = Cast<AGrabbeeWeapon>(HeldObject))
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
						
						// 先释放 Attach
						ReleaseAttach();
						
						// 通知物体被释放
						HeldObject->OnReleased(this);
						OnObjectReleased.Broadcast(HeldObject);
						
						// 存入背包（会销毁Actor）
						Inventory->TryStoreArrow();
						
						HeldObject = nullptr;
						bIsHolding = false;
						CurrentControlName = NAME_None;
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

AGrabbeeObject* UVRGrabHand::FindAngleClosestTarget()
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
		AGrabbeeObject::StaticClass(),
		IgnoreActors,
		OverlapActors
	);

	if (!bHit || OverlapActors.Num() == 0)
	{
		return nullptr;
	}

	// 找到角度最近的目标
	AGrabbeeObject* ClosestTarget = nullptr;
	float SmallestAngle = GravityGlovesAngle;

	FVector HandForward = GetForwardVector();

	for (AActor* Actor : OverlapActors)
	{
		AGrabbeeObject* GrabbeeObj = Cast<AGrabbeeObject>(Actor);
		if (!GrabbeeObj || !GrabbeeObj->CanBeGrabbedBy(this))
		{
			continue;
		}

		// 计算角度
		FVector ToTarget = (GrabbeeObj->GetActorLocation() - GetComponentLocation()).GetSafeNormal();
		float Angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(HandForward, ToTarget)));

		if (Angle < SmallestAngle)
		{
			SmallestAngle = Angle;
			ClosestTarget = GrabbeeObj;
		}
	}

	return ClosestTarget;
}

void UVRGrabHand::VirtualGrab(AGrabbeeObject* Target)
{
	if (!Target)
	{
		return;
	}

	// 设置虚拟抓取状态（与真实抓取相同的状态变量）
	HeldObject = Target;
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
	if (!bIsVirtualGrabbing || !HeldObject)
	{
		return;
	}

	AGrabbeeObject* ReleasedTarget = HeldObject;

	// 如果需要发射物体
	if (bLaunch)
	{
		ReleasedTarget->LaunchTowards(GetComponentLocation(), LaunchArcParam);
	}

	// 清除状态
	HeldObject = nullptr;
	bIsHolding = false;
	bIsVirtualGrabbing = false;
}

// ==================== 内部函数 ====================

void UVRGrabHand::UpdateGravityGloves(float DeltaTime)
{
	// 如果正在虚拟抓取，检测向后拉动手势
	if (bIsVirtualGrabbing && HeldObject)
	{
		// 检查目标是否仍然有效
		if (!IsValid(HeldObject))
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
		AGrabbeeObject* NewTarget = FindAngleClosestTarget();

		// 目标发生变化
		if (NewTarget != GravityGlovesTarget)
		{
			// 取消选中旧目标
			if (GravityGlovesTarget)
			{
				GravityGlovesTarget->OnDeselected();
			}

			// 选中新目标
			GravityGlovesTarget = NewTarget;
			if (GravityGlovesTarget)
			{
				GravityGlovesTarget->OnSelected();
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
	if (!HeldObject)
	{
		return false;
	}

	// 计算从物体指向手的方向
	FVector ToHand = (GetComponentLocation() - HeldObject->GetActorLocation()).GetSafeNormal();

	// 计算手部速度与该方向的点积（向后拉 = 负值）
	// 我们需要的是向后拉，即手部向着远离物体的方向移动
	// 但用户说的是"向后拉"，意思是手往自己身体方向拉，物体在前方
	// 所以应该是手部速度与 ToHand 方向的点积为负（手往物体相反方向移动）
	
	// 或者理解为：手往后拉 = 手速度与 -ToHand（指向物体的方向）点积为负
	// 即手速度与从手指向物体的方向点积为负
	FVector ToObject = -ToHand;
	float DotProduct = FVector::DotProduct(HandVelocity.GetSafeNormal(), ToObject);

	// 当点积为负且速度足够大时，触发发射
	// DotProduct < 0 表示手在远离物体
	return DotProduct < -PullBackThreshold && HandVelocity.Size() > 100.0f;
}

AGrabbeeObject* UVRGrabHand::PerformSphereOverlap() const
{
	TArray<AActor*> OverlapActors;
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(GetOwner());

	bool bHit = UKismetSystemLibrary::SphereOverlapActors(
		this,
		GetComponentLocation(),
		GrabSphereRadius,
		GrabObjectTypes,
		AGrabbeeObject::StaticClass(),
		IgnoreActors,
		OverlapActors
	);

	if (bHit && OverlapActors.Num() > 0)
	{
		// 返回第一个有效的可抓取物体
		for (AActor* Actor : OverlapActors)
		{
			if (AGrabbeeObject* GrabbeeObj = Cast<AGrabbeeObject>(Actor))
			{
				if (GrabbeeObj->CanBeGrabbedBy(const_cast<UVRGrabHand*>(this)))
				{
					return GrabbeeObj;
				}
			}
		}
	}

	return nullptr;
}

bool UVRGrabHand::IsInGravityGlovesAngle(AGrabbeeObject* Target) const
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
