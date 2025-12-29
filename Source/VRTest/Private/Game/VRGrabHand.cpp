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
}

void UVRGrabHand::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 更新 Gravity Gloves
	if (bEnableGravityGloves && !bIsHolding)
	{
		UpdateGravityGloves(DeltaTime);
	}
}

// ==================== 重写 ====================

AGrabbeeObject* UVRGrabHand::FindTarget_Implementation()
{
	// 优先检测近距离球形重叠
	if (AGrabbeeObject* NearTarget = PerformSphereOverlap())
	{
		return NearTarget;
	}

	// 如果有 Gravity Gloves 目标正在拉取中
	if (bIsGravityGlovesDragging && GravityGlovesTarget)
	{
		// 检查目标是否已经到达近距离范围
		float Distance = FVector::Dist(GetComponentLocation(), GravityGlovesTarget->GetActorLocation());
		if (Distance < GrabSphereRadius * 2.0f)
		{
			return GravityGlovesTarget;
		}
	}

	return nullptr;
}

bool UVRGrabHand::IsInBackpackArea_Implementation() const
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

void UVRGrabHand::StartGravityGlovesPull(AGrabbeeObject* Target)
{
	if (!Target)
	{
		return;
	}

	GravityGlovesTarget = Target;
	bIsGravityGlovesDragging = true;

	// 可以在这里添加视觉/音效反馈
}

void UVRGrabHand::StopGravityGlovesPull()
{
	GravityGlovesTarget = nullptr;
	bIsGravityGlovesDragging = false;
}

// ==================== 内部函数 ====================

void UVRGrabHand::UpdateGravityGloves(float DeltaTime)
{
	// 如果正在拉取
	if (bIsGravityGlovesDragging && GravityGlovesTarget)
	{
		// 检查目标是否仍然有效
		if (!IsValid(GravityGlovesTarget) || !GravityGlovesTarget->CanBeGrabbedBy(this))
		{
			StopGravityGlovesPull();
			return;
		}

		// 计算拉取方向
		FVector ToHand = GetComponentLocation() - GravityGlovesTarget->GetActorLocation();
		float Distance = ToHand.Size();

		if (Distance < GrabSphereRadius)
		{
			// 已到达，停止拉取
			StopGravityGlovesPull();
			return;
		}

		// 应用拉取力
		FVector PullDirection = ToHand.GetSafeNormal();
		FVector Velocity = PullDirection * GravityGlovesPullSpeed;

		if (UPrimitiveComponent* Primitive = GravityGlovesTarget->GetGrabPrimitive())
		{
			Primitive->SetPhysicsLinearVelocity(Velocity);
		}
	}
	else
	{
		// 寻找新的 Gravity Gloves 目标
		AGrabbeeObject* NewTarget = FindAngleClosestTarget();
		
		// 这里只设置目标，实际拉取需要通过输入触发
		// GravityGlovesTarget = NewTarget;
		// 可以添加高亮显示等视觉反馈
	}
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
