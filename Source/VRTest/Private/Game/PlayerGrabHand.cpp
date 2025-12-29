// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/PlayerGrabHand.h"
#include "Game/InventoryComponent.h"
#include "Grabbee/GrabbeeObject.h"
#include "Grabbee/GrabbeeWeapon.h"
#include "PhysicsControlComponent.h"
#include "PhysicsControlData.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"

UPlayerGrabHand::UPlayerGrabHand()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerGrabHand::BeginPlay()
{
	Super::BeginPlay();
}

void UPlayerGrabHand::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 更新 PhysicsControl 目标位置（如果有）
	if (bIsHolding && HeldObject && !CurrentControlName.IsNone())
	{
		if (UPhysicsControlComponent* PhysicsControl = GetPhysicsControlComponent())
		{
			FPhysicsControlTarget ControlTarget;
			
			// 根据抓取类型设置目标
			switch (HeldObject->GrabType)
			{
			case EGrabType::Free:
				// Free: 保持相对位置
				ControlTarget.TargetPosition = GetComponentLocation();
				ControlTarget.TargetOrientation = GetComponentRotation();
				PhysicsControl->SetControlTarget(CurrentControlName, ControlTarget, true);
				break;
			case EGrabType::Snap:
				// Snap: 对齐到目标位置
				{
					FTransform TargetTransform = HeldObject->SnapOffset * GetComponentTransform();
					ControlTarget.TargetPosition = TargetTransform.GetLocation();
					ControlTarget.TargetOrientation = TargetTransform.Rotator();
					PhysicsControl->SetControlTarget(CurrentControlName, ControlTarget, true);
				}
				break;
			case EGrabType::HumanBody:
				// HumanBody: 跟随手部
				ControlTarget.TargetPosition = GetComponentLocation();
				ControlTarget.TargetOrientation = GetComponentRotation();
				PhysicsControl->SetControlTarget(CurrentControlName, ControlTarget, true);
				break;
			default:
				break;
			}
		}
	}
}

// ==================== 核心接口 ====================

void UPlayerGrabHand::TryGrab(bool bFromBackpack)
{
	if (bIsHolding)
	{
		return;
	}

	AGrabbeeObject* Target = nullptr;

	if (bFromBackpack)
	{
		// 从背包取箭
		if (UInventoryComponent* Inventory = GetInventoryComponent())
		{
			Target = Inventory->TryRetrieveArrow(GetComponentTransform());
		}
	}
	else
	{
		// 检测抓取目标
		Target = FindTarget();
	}

	if (Target && ValidateGrab(Target))
	{
		GrabObject(Target);
	}
}

void UPlayerGrabHand::TryRelease(bool bToBackpack)
{
	if (!bIsHolding || !HeldObject)
	{
		return;
	}

	if (!ValidateRelease())
	{
		return;
	}

	if (bToBackpack)
	{
		// 只有箭可以放入背包
		if (AGrabbeeWeapon* Weapon = Cast<AGrabbeeWeapon>(HeldObject))
		{
			if (Weapon->WeaponType == EWeaponType::Arrow)
			{
				if (UInventoryComponent* Inventory = GetInventoryComponent())
				{
					if (Inventory->TryStoreArrow())
					{
						// 先释放物理/Attach
						ReleasePhysicsControl();
						ReleaseAttach();
						
						// 通知物体被释放
						HeldObject->OnReleased(this);
						OnObjectReleased.Broadcast(HeldObject);
						
						// 销毁物体
						HeldObject->Destroy();
						HeldObject = nullptr;
						bIsHolding = false;
						CurrentControlName = NAME_None;
						return;
					}
				}
			}
		}
	}

	// 正常释放
	ReleaseObject();
}

AGrabbeeObject* UPlayerGrabHand::FindTarget_Implementation()
{
	// 基类不实现，由子类重写
	return nullptr;
}

bool UPlayerGrabHand::IsInBackpackArea_Implementation() const
{
	// 基类返回 false，VR 子类重写
	return false;
}

// ==================== 抓取实现 ====================

void UPlayerGrabHand::GrabObject(AGrabbeeObject* Target)
{
	if (!Target)
	{
		return;
	}

	// 处理另一只手持有的情况
	HandleOtherHandHolding(Target);

	// 根据抓取类型执行不同逻辑
	switch (Target->GrabType)
	{
	case EGrabType::Free:
		GrabFree(Target);
		break;
	case EGrabType::Snap:
		GrabSnap(Target);
		break;
	case EGrabType::WeaponSnap:
		if (AGrabbeeWeapon* Weapon = Cast<AGrabbeeWeapon>(Target))
		{
			GrabWeaponSnap(Weapon);
		}
        else
        {
            // 非武器类型错误处理
            UE_LOG(LogTemp, Warning, TEXT("Attempted to grab a WeaponSnap object that is not a weapon."));
            return;
        }
		break;
	case EGrabType::HumanBody:
		GrabHumanBody(Target);
		break;
	case EGrabType::Custom:
		GrabCustom(Target);
		break;
	default:
		return;
	}

	// 更新状态
	HeldObject = Target;
	bIsHolding = true;

	// 通知物体被抓取
	Target->OnGrabbed(this);

	// 广播委托
	OnObjectGrabbed.Broadcast(Target);
}

void UPlayerGrabHand::ReleaseObject()
{
	if (!HeldObject)
	{
		return;
	}

	AGrabbeeObject* ReleasedObj = HeldObject;
	EGrabType GrabType = ReleasedObj->GrabType;

	// 根据抓取类型执行释放
	switch (GrabType)
	{
	case EGrabType::Free:
	case EGrabType::Snap:
	case EGrabType::HumanBody:
		ReleasePhysicsControl();
		break;
	case EGrabType::WeaponSnap:
		ReleaseAttach();
		break;
	case EGrabType::Custom:
		ReleasedObj->CustomRelease(this);
		break;
	default:
		break;
	}

	// 通知物体被释放
	ReleasedObj->OnReleased(this);

	// 更新状态
	HeldObject = nullptr;
	bIsHolding = false;
	CurrentControlName = NAME_None;

	// 广播委托
	OnObjectReleased.Broadcast(ReleasedObj);
}

// ==================== 内部实现 ====================

void UPlayerGrabHand::GrabFree(AGrabbeeObject* Target)
{
	UPhysicsControlComponent* PhysicsControl = GetPhysicsControlComponent();
	if (!PhysicsControl)
	{
		return;
	}

	UPrimitiveComponent* Primitive = Target->GetGrabPrimitive();
	if (!Primitive)
	{
		return;
	}

	// 准备控制数据
	FPhysicsControlData ControlData;
	ControlData.LinearStrength = FreeGrabStrength;
	ControlData.LinearDampingRatio = FreeGrabDamping;
	ControlData.AngularStrength = FreeGrabStrength;
	ControlData.AngularDampingRatio = FreeGrabDamping;

	// 准备初始目标
	FPhysicsControlTarget ControlTarget;
	ControlTarget.TargetPosition = GetComponentLocation();
	ControlTarget.TargetOrientation = GetComponentRotation();

	// 尝试从 StaticMesh 创建控制
	UMeshComponent* MeshComp = Cast<UMeshComponent>(Primitive);
	if (MeshComp)
	{
		// 使用 CreateControl（父组件为空表示世界空间控制）
		CurrentControlName = PhysicsControl->CreateControl(
			nullptr,        // ParentMeshComponent - 无父级，世界空间
			NAME_None,      // ParentBoneName
			MeshComp,       // ChildMeshComponent
			NAME_None,      // ChildBoneName
			ControlData,
			ControlTarget,
			NAME_None       // Set
		);
	}
}

void UPlayerGrabHand::GrabSnap(AGrabbeeObject* Target)
{
	UPhysicsControlComponent* PhysicsControl = GetPhysicsControlComponent();
	if (!PhysicsControl)
	{
		return;
	}

	UPrimitiveComponent* Primitive = Target->GetGrabPrimitive();
	if (!Primitive)
	{
		return;
	}

	// 准备控制数据（更高强度以实现快速对齐）
	FPhysicsControlData ControlData;
	ControlData.LinearStrength = SnapGrabStrength;
	ControlData.LinearDampingRatio = SnapGrabDamping;
	ControlData.AngularStrength = SnapGrabStrength;
	ControlData.AngularDampingRatio = SnapGrabDamping;

	// 计算 Snap 目标位置
	FTransform TargetTransform = Target->SnapOffset * GetComponentTransform();
	
	FPhysicsControlTarget ControlTarget;
	ControlTarget.TargetPosition = TargetTransform.GetLocation();
	ControlTarget.TargetOrientation = TargetTransform.Rotator();

	UMeshComponent* MeshComp = Cast<UMeshComponent>(Primitive);
	if (MeshComp)
	{
		CurrentControlName = PhysicsControl->CreateControl(
			nullptr,
			NAME_None,
			MeshComp,
			NAME_None,
			ControlData,
			ControlTarget,
			NAME_None
		);
	}
}

void UPlayerGrabHand::GrabWeaponSnap(AGrabbeeWeapon* Weapon)
{
	if (!Weapon)
	{
		return;
	}

	// 查找此武器类型的偏移
	FTransform* Offset = WeaponGrabOffsets.Find(Weapon->WeaponType);
	FTransform FinalOffset = Offset ? *Offset : FTransform::Identity;

	// 计算目标变换
	FTransform TargetTransform = FinalOffset * GetComponentTransform();

	// 设置物体位置并附加
	Weapon->SetActorTransform(TargetTransform);
	Weapon->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);

	// 禁用物理模拟
	Weapon->SetSimulatePhysics(false);
}

void UPlayerGrabHand::GrabHumanBody(AGrabbeeObject* Target)
{
	// HumanBody 逻辑与 Free 类似，但可能需要指定骨骼
	// 目前使用与 Free 相同的实现
	GrabFree(Target);
}

void UPlayerGrabHand::GrabCustom(AGrabbeeObject* Target)
{
	// 调用物体的自定义抓取函数
	Target->CustomGrab(this);
}

void UPlayerGrabHand::ReleasePhysicsControl()
{
	if (CurrentControlName.IsNone())
	{
		return;
	}

	if (UPhysicsControlComponent* PhysicsControl = GetPhysicsControlComponent())
	{
		PhysicsControl->DestroyControl(CurrentControlName);
	}

	CurrentControlName = NAME_None;
}

void UPlayerGrabHand::ReleaseAttach()
{
	if (HeldObject)
	{
		HeldObject->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		HeldObject->SetSimulatePhysics(true);
	}
}

void UPlayerGrabHand::ReleaseAttachOnly()
{
	if (!HeldObject)
	{
		return;
	}

	// 释放附加但不触发 OnReleased
	HeldObject->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	HeldObject->SetSimulatePhysics(true);

	// 更新状态
	HeldObject = nullptr;
	bIsHolding = false;
	CurrentControlName = NAME_None;
}

// ==================== 辅助函数 ====================

UInventoryComponent* UPlayerGrabHand::GetInventoryComponent() const
{
	if (AActor* Owner = GetOwner())
	{
		return Owner->FindComponentByClass<UInventoryComponent>();
	}
	return nullptr;
}

UPhysicsControlComponent* UPlayerGrabHand::GetPhysicsControlComponent() const
{
	if (AActor* Owner = GetOwner())
	{
		return Owner->FindComponentByClass<UPhysicsControlComponent>();
	}
	return nullptr;
}

FName UPlayerGrabHand::GenerateControlName() const
{
	static int32 ControlCounter = 0;
	return FName(*FString::Printf(TEXT("GrabControl_%s_%d"), bIsRightHand ? TEXT("R") : TEXT("L"), ControlCounter++));
}

bool UPlayerGrabHand::ValidateGrab(AGrabbeeObject* Target) const
{
	if (!Target)
	{
		return false;
	}

	return Target->CanBeGrabbedBy(const_cast<UPlayerGrabHand*>(this));
}

bool UPlayerGrabHand::ValidateRelease() const
{
	return bIsHolding && HeldObject != nullptr;
}

void UPlayerGrabHand::HandleOtherHandHolding(AGrabbeeObject* Target)
{
	if (!OtherHand || !Target)
	{
		return;
	}

	// 如果另一只手持有同一物体，先释放
	if (OtherHand->HeldObject == Target)
	{
		OtherHand->ReleaseObject();
	}
}
