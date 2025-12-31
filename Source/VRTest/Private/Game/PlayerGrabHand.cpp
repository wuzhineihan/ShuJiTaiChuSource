// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/PlayerGrabHand.h"
#include "Game/InventoryComponent.h"
#include "Grab/IGrabbable.h"
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

	// 缓存组件引用以优化性能
	CachedPhysicsControl = GetPhysicsControlComponent();
	CachedInventory = GetInventoryComponent();
}

void UPlayerGrabHand::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 清理 PhysicsControl 避免悬空引用
	if (!CurrentControlName.IsNone() && CachedPhysicsControl)
	{
		CachedPhysicsControl->DestroyControl(CurrentControlName);
		CurrentControlName = NAME_None;
	}

	// 清空缓存的组件引用
	CachedPhysicsControl = nullptr;
	CachedInventory = nullptr;

	Super::EndPlay(EndPlayReason);
}

void UPlayerGrabHand::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 更新 PhysicsControl 目标位置（如果有）
	if (bIsHolding && HeldActor && !CurrentControlName.IsNone())
	{
		if (CachedPhysicsControl)
		{
			FPhysicsControlTarget ControlTarget;
			
			// 根据缓存的抓取类型设置目标
			switch (HeldGrabType)
			{
			case EGrabType::Free:
				// Free: 保持抓取时的相对位置
				{
					FTransform TargetTransform = GetComponentTransform() * GrabOffset;
					ControlTarget.TargetPosition = TargetTransform.GetLocation();
					ControlTarget.TargetOrientation = TargetTransform.Rotator();
					CachedPhysicsControl->SetControlTarget(CurrentControlName, ControlTarget, true);
				}
				break;
			case EGrabType::Snap:
				// Snap: 对齐到预设的目标位置（GrabOffset 在 GrabSnap 时已保存）
				{
					FTransform TargetTransform = GetComponentTransform() * GrabOffset;
					ControlTarget.TargetPosition = TargetTransform.GetLocation();
					ControlTarget.TargetOrientation = TargetTransform.Rotator();
					CachedPhysicsControl->SetControlTarget(CurrentControlName, ControlTarget, true);
				}
				break;
			case EGrabType::HumanBody:
				// HumanBody: 跟随手部
				ControlTarget.TargetPosition = GetComponentLocation();
				ControlTarget.TargetOrientation = GetComponentRotation();
				CachedPhysicsControl->SetControlTarget(CurrentControlName, ControlTarget, true);
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
	// Step 1: 有效性检验
	if (bIsHolding)
	{
		return;
	}

	// Step 2: 查找抓取目标
	FName BoneName = NAME_None;
	AActor* TargetActor = FindTarget(bFromBackpack, BoneName);
	
	if (!TargetActor)
	{
		return;
	}

	// Step 3: 获取接口并验证
	IGrabbable* Grabbable = Cast<IGrabbable>(TargetActor);
	if (!Grabbable)
	{
		return;
	}

	// Step 4: 验证是否可抓取
	if (ValidateGrab(Grabbable))
	{
		GrabObject(TargetActor, BoneName);
	}
}

void UPlayerGrabHand::TryRelease(bool bToBackpack)
{
	// ValidateRelease 内部处理所有有效性检查
	if (!ValidateRelease())
	{
		return;
	}

	if (bToBackpack)
	{
		// 只有箭可以放入背包
		if (AGrabbeeWeapon* Weapon = Cast<AGrabbeeWeapon>(HeldActor))
		{
			if (Weapon->WeaponType == EWeaponType::Arrow)
			{
				if (CachedInventory)
				{
					if (CachedInventory->TryStoreArrow())
					{
						// 先释放物理/Attach
						ReleasePhysicsControl();
						ReleaseAttach();
						
						// 通知物体被释放（通过接口）
						if (IGrabbable* Grabbable = Cast<IGrabbable>(HeldActor))
						{
							IGrabbable::Execute_OnReleased(HeldActor, this);
						}
						OnObjectReleased.Broadcast(HeldActor);
						
						// 销毁物体
						HeldActor->Destroy();
						HeldActor = nullptr;
						HeldGrabType = EGrabType::None;
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

AActor* UPlayerGrabHand::FindTarget(bool bFromBackpack, FName& OutBoneName)
{
	OutBoneName = NAME_None;
	
	// 优先从背包取物
	if (bFromBackpack)
	{
		if (CachedInventory)
		{
			AActor* ArrowFromBackpack = CachedInventory->TryRetrieveArrow(GetComponentTransform());
			if (ArrowFromBackpack)
			{
				return ArrowFromBackpack;
			}
		}
	}

	// 基类不实现物理检测，由子类重写
	return nullptr;
}


// ==================== 抓取实现 ====================

void UPlayerGrabHand::GrabObject(AActor* TargetActor, FName BoneName)
{
	if (!TargetActor)
	{
		return;
	}

	// 获取接口
	IGrabbable* Grabbable = Cast<IGrabbable>(TargetActor);
	if (!Grabbable)
	{
		UE_LOG(LogTemp, Warning, TEXT("GrabObject: Target does not implement IGrabbable"));
		return;
	}

	// 获取抓取类型
	EGrabType GrabType = IGrabbable::Execute_GetGrabType(TargetActor);

	// 处理另一只手持有的情况（仅对非双手抓取物体）
	bool bSupportsDual = IGrabbable::Execute_SupportsDualHandGrab(TargetActor);
	if (!bSupportsDual)
	{
		HandleOtherHandHolding(TargetActor, Grabbable);
	}

	// 保存骨骼名和抓取类型
	GrabbedBoneName = BoneName;
	HeldGrabType = GrabType;

	// 根据抓取类型执行不同逻辑
	switch (GrabType)
	{
	case EGrabType::Free:
		GrabFree(Grabbable, TargetActor);
		break;
	case EGrabType::Snap:
		GrabSnap(Grabbable, TargetActor);
		break;
	case EGrabType::WeaponSnap:
		if (AGrabbeeWeapon* Weapon = Cast<AGrabbeeWeapon>(TargetActor))
		{
			GrabWeaponSnap(Weapon);
		}
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Attempted to grab a WeaponSnap object that is not a weapon."));
            return;
        }
		break;
	case EGrabType::HumanBody:
		GrabHumanBody(Grabbable, TargetActor, BoneName);
		break;
	case EGrabType::Custom:
		// 啥都不做，自定义逻辑放在OnGrabbed里面实现
		break;
	default:
		return;
	}

	// 更新状态
	HeldActor = TargetActor;
	bIsHolding = true;

	// 通知物体被抓取（通过接口）
	IGrabbable::Execute_OnGrabbed(TargetActor, this);

	// 广播委托
	OnObjectGrabbed.Broadcast(TargetActor);
}

void UPlayerGrabHand::ReleaseObject()
{
	if (!HeldActor)
	{
		return;
	}

	AActor* ReleasedActor = HeldActor;
	EGrabType GrabType = HeldGrabType;

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
		// Custom 类型不做额外物理处理
		break;
	default:
		break;
	}

	// 通知物体被释放（通过接口）
	if (IGrabbable* Grabbable = Cast<IGrabbable>(ReleasedActor))
	{
		IGrabbable::Execute_OnReleased(ReleasedActor, this);
	}

	// 更新状态
	HeldActor = nullptr;
	HeldGrabType = EGrabType::None;
	bIsHolding = false;
	CurrentControlName = NAME_None;
	GrabbedBoneName = NAME_None;

	// 广播委托
	OnObjectReleased.Broadcast(ReleasedActor);
}

void UPlayerGrabHand::ReleaseAttachOnly()
{
	if (!HeldActor)
	{
		return;
	}

	// 释放附加但不触发 OnReleased
	HeldActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	
	// 尝试恢复物理模拟
	if (IGrabbable* Grabbable = Cast<IGrabbable>(HeldActor))
	{
		if (UPrimitiveComponent* Primitive = IGrabbable::Execute_GetGrabPrimitive(HeldActor))
		{
			Primitive->SetSimulatePhysics(true);
		}
	}

	// 更新状态
	HeldActor = nullptr;
	HeldGrabType = EGrabType::None;
	bIsHolding = false;
	CurrentControlName = NAME_None;
}

// ==================== 内部实现 ====================

void UPlayerGrabHand::GrabFree(IGrabbable* Grabbable, AActor* TargetActor)
{
	if (!CachedPhysicsControl || !Grabbable || !TargetActor)
	{
		return;
	}

	UPrimitiveComponent* Primitive = IGrabbable::Execute_GetGrabPrimitive(TargetActor);
	if (!Primitive)
	{
		return;
	}

	// 记录抓取时的相对变换（物体相对于手）
	FTransform HandTransform = GetComponentTransform();
	FTransform ObjectTransform = TargetActor->GetActorTransform();
	GrabOffset = ObjectTransform.GetRelativeTransform(HandTransform);

	// 准备控制数据
	FPhysicsControlData ControlData;
	ControlData.LinearStrength = FreeGrabStrength;
	ControlData.LinearDampingRatio = FreeGrabDamping;
	ControlData.AngularStrength = FreeGrabStrength;
	ControlData.AngularDampingRatio = FreeGrabDamping;

	// 准备初始目标
	FTransform TargetTransform = HandTransform * GrabOffset;
	FPhysicsControlTarget ControlTarget;
	ControlTarget.TargetPosition = TargetTransform.GetLocation();
	ControlTarget.TargetOrientation = TargetTransform.Rotator();

	UMeshComponent* MeshComp = Cast<UMeshComponent>(Primitive);
	if (MeshComp)
	{
		CurrentControlName = CachedPhysicsControl->CreateControl(
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

void UPlayerGrabHand::GrabSnap(IGrabbable* Grabbable, AActor* TargetActor)
{
	if (!CachedPhysicsControl || !Grabbable || !TargetActor)
	{
		return;
	}

	UPrimitiveComponent* Primitive = IGrabbable::Execute_GetGrabPrimitive(TargetActor);
	if (!Primitive)
	{
		return;
	}

	// 获取 SnapOffset（通过接口）
	FTransform SnapOffset = IGrabbable::Execute_GetSnapOffset(TargetActor);
	GrabOffset = SnapOffset;

	// 准备控制数据
	FPhysicsControlData ControlData;
	ControlData.LinearStrength = SnapGrabStrength;
	ControlData.LinearDampingRatio = SnapGrabDamping;
	ControlData.AngularStrength = SnapGrabStrength;
	ControlData.AngularDampingRatio = SnapGrabDamping;

	// 计算 Snap 目标位置
	FTransform TargetTransform = GetComponentTransform() * SnapOffset;
	FPhysicsControlTarget ControlTarget;
	ControlTarget.TargetPosition = TargetTransform.GetLocation();
	ControlTarget.TargetOrientation = TargetTransform.Rotator();

	UMeshComponent* MeshComp = Cast<UMeshComponent>(Primitive);
	if (MeshComp)
	{
		CurrentControlName = CachedPhysicsControl->CreateControl(
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
	FTransform TargetTransform = GetComponentTransform() * FinalOffset;

	// 设置物体位置并附加
	Weapon->SetActorTransform(TargetTransform);
	Weapon->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);

	// 禁用物理模拟
	Weapon->SetSimulatePhysics(false);
}

void UPlayerGrabHand::GrabHumanBody(IGrabbable* Grabbable, AActor* TargetActor, FName BoneName)
{
	if (!CachedPhysicsControl || !Grabbable || !TargetActor)
	{
		return;
	}

	UPrimitiveComponent* Primitive = IGrabbable::Execute_GetGrabPrimitive(TargetActor);
	if (!Primitive)
	{
		return;
	}

	// 尝试获取骨骼网格体组件
	USkeletalMeshComponent* SkelMesh = Cast<USkeletalMeshComponent>(Primitive);
	
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

	// 生成独立的控制名
	FString ControlNameStr = FString::Printf(TEXT("DragBody_%s_%s"), 
		bIsRightHand ? TEXT("Right") : TEXT("Left"),
		*TargetActor->GetName());
	FName GeneratedControlName = FName(*ControlNameStr);

	// 如果目标是骨骼网格体且有有效的骨骼名
	if (SkelMesh && !BoneName.IsNone())
	{
		int32 BoneIndex = SkelMesh->GetBoneIndex(BoneName);
		if (BoneIndex != INDEX_NONE)
		{
			CurrentControlName = CachedPhysicsControl->CreateControl(
				nullptr,
				NAME_None,
				SkelMesh,
				BoneName,
				ControlData,
				ControlTarget,
				GeneratedControlName
			);
			
			UE_LOG(LogTemp, Log, TEXT("GrabHumanBody: Created control '%s' for bone '%s'"), 
				*CurrentControlName.ToString(), *BoneName.ToString());
			return;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("GrabHumanBody: Bone '%s' not found in mesh, using root"), 
				*BoneName.ToString());
		}
	}

	// 回退：使用普通网格体控制
	UMeshComponent* MeshComp = Cast<UMeshComponent>(Primitive);
	if (MeshComp)
	{
		CurrentControlName = CachedPhysicsControl->CreateControl(
			nullptr,
			NAME_None,
			MeshComp,
			NAME_None,
			ControlData,
			ControlTarget,
			GeneratedControlName
		);
		
		UE_LOG(LogTemp, Log, TEXT("GrabHumanBody: Created control '%s' without bone"), 
			*CurrentControlName.ToString());
	}
}

void UPlayerGrabHand::ReleasePhysicsControl()
{
	if (CurrentControlName.IsNone())
	{
		return;
	}

	if (CachedPhysicsControl)
	{
		CachedPhysicsControl->DestroyControl(CurrentControlName);
	}

	CurrentControlName = NAME_None;
}

void UPlayerGrabHand::ReleaseAttach()
{
	if (HeldActor)
	{
		HeldActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		
		// 恢复物理模拟
		if (IGrabbable* Grabbable = Cast<IGrabbable>(HeldActor))
		{
			if (UPrimitiveComponent* Primitive = IGrabbable::Execute_GetGrabPrimitive(HeldActor))
			{
				Primitive->SetSimulatePhysics(true);
			}
		}
	}
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

bool UPlayerGrabHand::ValidateGrab(IGrabbable* Grabbable) const
{
	if (!Grabbable)
	{
		return false;
	}

	// 通过接口调用 CanBeGrabbedBy（需要获取 Actor）
	AActor* GrabbableActor = Cast<AActor>(Grabbable);
	if (!GrabbableActor)
	{
		return false;
	}

	return IGrabbable::Execute_CanBeGrabbedBy(GrabbableActor, this);
}

bool UPlayerGrabHand::ValidateRelease() const
{
	return bIsHolding && HeldActor != nullptr;
}

void UPlayerGrabHand::HandleOtherHandHolding(AActor* TargetActor, IGrabbable* Grabbable)
{
	if (!OtherHand || !TargetActor)
	{
		return;
	}

	// 如果另一只手持有同一物体，先释放
	if (OtherHand->HeldActor == TargetActor)
	{
		OtherHand->ReleaseObject();
	}
}
