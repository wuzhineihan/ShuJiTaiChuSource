// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/PlayerGrabHand.h"
#include "Game/InventoryComponent.h"
#include "Grab/IGrabbable.h"
#include "Grabbee/GrabbeeWeapon.h"
#include "PhysicsControlComponent.h"
#include "PhysicsControlData.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"

UPlayerGrabHand::UPlayerGrabHand()
{
	PrimaryComponentTick.bCanEverTick = true;

	// 创建手部碰撞体
	HandCollision = CreateDefaultSubobject<USphereComponent>(TEXT("HandCollision"));
	HandCollision->SetupAttachment(this);
	HandCollision->SetSphereRadius(5.0f);
	HandCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HandCollision->SetCollisionResponseToAllChannels(ECR_Overlap);
	HandCollision->ComponentTags.Add(FName("player_hand"));

	// 默认检测对象类型
	GrabObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	GrabObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody));
}

void UPlayerGrabHand::BeginPlay()
{
	Super::BeginPlay();

	// 缓存组件引用以优化性能
	if (AActor* Owner = GetOwner())
	{
		CachedPhysicsControl = Owner->FindComponentByClass<UPhysicsControlComponent>();
		CachedInventory = Owner->FindComponentByClass<UInventoryComponent>();
	}
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
			case EGrabType::WeaponSnap:
				// WeaponSnap: 对齐到武器偏移位置
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

	// Step 4: 抓取，统一在GrabObject里进行验证
	GrabObject(TargetActor, BoneName);
}

void UPlayerGrabHand::TryRelease(bool bToBackpack)
{
	// ValidateRelease 内部处理所有有效性检查
	if (!(bIsHolding && HeldActor != nullptr))
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
				if (CachedInventory && CachedInventory->TryStoreArrow())
				{
					// 保存指针用于销毁
					AActor* ArrowToDestroy = HeldActor;
					
					// 统一通过 ReleaseObject 释放（处理物理控制、状态清理、回调）
					ReleaseObject();
					
					// 销毁箭 Actor
					ArrowToDestroy->Destroy();
					return;
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

	// 在 GrabObject 开头统一检查 CanBeGrabbedBy
	if (!IGrabbable::Execute_CanBeGrabbedBy(TargetActor, this))
	{
		UE_LOG(LogTemp, Warning, TEXT("GrabObject: Target cannot be grabbed by this hand"));
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
		GrabFree(TargetActor);
		break;
	case EGrabType::WeaponSnap:
		GrabWeaponSnap(TargetActor);
		break;
	case EGrabType::HumanBody:
		GrabHumanBody(TargetActor, BoneName);
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

	// 根据抓取类型执行释放 - 现在所有类型都使用 PhysicsControl
	switch (GrabType)
	{
	case EGrabType::Free:
	case EGrabType::WeaponSnap:
	case EGrabType::HumanBody:
		ReleasePhysicsControl();
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

// ==================== 内部实现 ====================

void UPlayerGrabHand::GrabFree(AActor* TargetActor)
{
	if (!CachedPhysicsControl || !TargetActor)
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

void UPlayerGrabHand::GrabWeaponSnap(AActor* TargetActor)
{
	if (!CachedPhysicsControl || !TargetActor)
	{
		return;
	}

	AGrabbeeWeapon* Weapon = Cast<AGrabbeeWeapon>(TargetActor);
	if (!Weapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("GrabWeaponSnap: Target is not a weapon"));
		return;
	}

	// 查找此武器类型的偏移
	FTransform* Offset = WeaponGrabOffsets.Find(Weapon->WeaponType);
	FTransform FinalOffset = Offset ? *Offset : FTransform::Identity;
	GrabOffset = FinalOffset;

	// 获取 Primitive
	UPrimitiveComponent* Primitive = IGrabbable::Execute_GetGrabPrimitive(Weapon);
	if (!Primitive)
	{
		return;
	}

	// 暂时禁用物理，以便直接设置位置
	Primitive->SetSimulatePhysics(false);

	// 计算目标变换
	FTransform TargetTransform = GetComponentTransform() * FinalOffset;

	// 瞬移武器到手部位置
	Weapon->SetActorTransform(TargetTransform);

	// 准备控制数据 - 使用高强度使武器紧跟手部
	FPhysicsControlData ControlData;
	ControlData.LinearStrength = WeaponSnapStrength;
	ControlData.LinearDampingRatio = WeaponSnapDamping;
	ControlData.AngularStrength = WeaponSnapStrength;
	ControlData.AngularDampingRatio = WeaponSnapDamping;

	// 准备初始目标
	FPhysicsControlTarget ControlTarget;
	ControlTarget.TargetPosition = TargetTransform.GetLocation();
	ControlTarget.TargetOrientation = TargetTransform.Rotator();

	UMeshComponent* MeshComp = Cast<UMeshComponent>(Primitive);
	if (MeshComp)
	{
		// 重新启用物理（PhysicsControl 需要物理模拟）
		Primitive->SetSimulatePhysics(true);
		
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

void UPlayerGrabHand::GrabHumanBody(AActor* TargetActor, FName BoneName)
{
	if (!CachedPhysicsControl || !TargetActor)
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
				NAME_None
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

// ==================== 辅助函数 ====================

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
