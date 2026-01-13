// Fill out your copyright notice in the Description page of Project Settings.

#include "Grabber/PlayerGrabHand.h"
#include "Game/InventoryComponent.h"
#include "Grabber/IGrabbable.h"
#include "Grabbee/GrabbeeWeapon.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Game/BasePlayer.h"

UPlayerGrabHand::UPlayerGrabHand()
{
	PrimaryComponentTick.bCanEverTick = true;

	// 默认检测对象类型
	GrabObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	GrabObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody));
	
	// HandCollision 由 BaseVRPlayer 创建并赋值
}

void UPlayerGrabHand::BeginPlay()
{
	Super::BeginPlay();
	
	PlayerCharacter = Cast<ABasePlayer>(GetOwner());
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerGrabHand: Owner is not ABasePlayer"));
		return;
	}
	// CachedPhysicsHandle 和 CachedInventory 将由 BasePlayer 在其 BeginPlay 中设置
}

void UPlayerGrabHand::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 确保释放 PhysicsHandle
	ReleasePhysicsHandle();

	// 清空缓存的组件引用
	CachedPhysicsHandle = nullptr;
	CachedInventory = nullptr;

	Super::EndPlay(EndPlayReason);
}

void UPlayerGrabHand::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 更新 PhysicsHandle 目标位置（如果正在抓取）
	if (bIsHolding && HeldActor && CachedPhysicsHandle && CachedPhysicsHandle->GrabbedComponent)
	{
		// 根据缓存的抓取类型设置目标
		switch (HeldGrabType)
		{
		case EGrabType::Free:
		case EGrabType::WeaponSnap:
			{
				// 将局部偏移转换为世界空间
				FTransform CurrentHandTransform = GetComponentTransform();
				FVector TargetPosition = CurrentHandTransform.TransformPosition(GrabOffset.GetLocation());
				FRotator TargetRotation = (CurrentHandTransform.GetRotation() * GrabOffset.GetRotation()).Rotator();
				
				CachedPhysicsHandle->SetTargetLocationAndRotation(TargetPosition, TargetRotation);
			}
			break;
		case EGrabType::HumanBody:
			// HumanBody: 直接跟随手部位置
			CachedPhysicsHandle->SetTargetLocationAndRotation(GetComponentLocation(), GetComponentRotation());
			break;
		default:
			break;
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
	if (!TargetActor || !CachedPhysicsHandle)
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

	// 获取 Primitive（所有物理抓取类型都需要）
	UPrimitiveComponent* Primitive = IGrabbable::Execute_GetGrabPrimitive(TargetActor);
	if (!Primitive && IGrabbable::Execute_GetGrabType(TargetActor) != EGrabType::Custom)
	{
		UE_LOG(LogTemp, Warning, TEXT("GrabObject: Target's grab primitive is null"));
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

	// ==================== 根据抓取类型处理特定逻辑 ====================
	FVector GrabLocation;
	FRotator GrabRotation;
	FName GrabBoneName = NAME_None;
	bool bUseSnapStrength = false;

	switch (GrabType)
	{
	case EGrabType::Free:
		{
			// 计算物体相对于手的局部偏移
			FTransform HandTransform = GetComponentTransform();
			FTransform ObjectTransform = TargetActor->GetActorTransform();

			GrabLocation = Primitive->GetCenterOfMass(GrabBoneName);
			GrabRotation = ObjectTransform.Rotator();
			
			FVector LocalOffset = HandTransform.InverseTransformPosition(GrabLocation);
			FQuat LocalRotation = HandTransform.GetRotation().Inverse() * FQuat(GrabRotation);
			GrabOffset = FTransform(LocalRotation, LocalOffset, FVector::OneVector);
		}
		break;

	case EGrabType::WeaponSnap:
		{
			AGrabbeeWeapon* Weapon = Cast<AGrabbeeWeapon>(TargetActor);
			if (!Weapon)
			{
				UE_LOG(LogTemp, Warning, TEXT("GrabObject: WeaponSnap grab type requires AGrabbeeWeapon"));
				return;
			}

			if (Weapon->WeaponType == EWeaponType::Bow)
			{
				// 通知玩家角色首次拾取弓
				if (PlayerCharacter && PlayerCharacter->CheckBowFirstPickedUp())
				{
					TargetActor->Destroy();
					return;
				}
			}
			
			// 查找此武器类型的偏移
			FTransform* Offset = WeaponGrabOffsets.Find(Weapon->WeaponType);
			GrabOffset = Offset ? *Offset : FTransform::Identity;

			// 瞬移武器到手部位置
			Primitive->SetSimulatePhysics(false);
			
			FTransform HandTransform = GetComponentTransform();
			FVector TargetPosition = HandTransform.TransformPosition(GrabOffset.GetLocation());
			FRotator TargetRotation = (HandTransform.GetRotation() * GrabOffset.GetRotation()).Rotator();
			
			Weapon->SetActorLocationAndRotation(TargetPosition, TargetRotation);
			Primitive->SetSimulatePhysics(true);
			

			// GrabLocation 是物体上的抓取点（质心），不是目标位置
			GrabLocation = Primitive->GetComponentLocation();
			GrabRotation = Primitive->GetComponentRotation();

			bUseSnapStrength = true;
		}
		break;

	case EGrabType::HumanBody:
		{
			// 如果有骨骼名则使用骨骼位置
			if (USkeletalMeshComponent* SkelMesh = Cast<USkeletalMeshComponent>(Primitive))
			{
				if (!BoneName.IsNone() && SkelMesh->GetBoneIndex(BoneName) != INDEX_NONE)
				{
					GrabBoneName = BoneName;
                 	GrabLocation = SkelMesh->GetCenterOfMass(GrabBoneName);
					GrabRotation = GetComponentRotation();
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("GrabObject: HumanBody grab type requires valid bone name"));
					return;
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("GrabObject: HumanBody grab type requires SkeletalMeshComponent"));
				return;
			}
		}
		break;

	case EGrabType::Custom:
		// Custom 类型不使用 PhysicsHandle，跳过物理抓取逻辑
		break;

	default:
		return;
	}

	// ==================== 共同逻辑：配置 PhysicsHandle 并执行抓取 ====================
	if (GrabType != EGrabType::Custom)
	{
		// 配置 PhysicsHandle 参数
		if (bUseSnapStrength)
		{
			CachedPhysicsHandle->LinearDamping = WeaponSnapLinearDamping;
			CachedPhysicsHandle->LinearStiffness = WeaponSnapLinearStiffness;
			CachedPhysicsHandle->AngularDamping = WeaponSnapAngularDamping;
			CachedPhysicsHandle->AngularStiffness = WeaponSnapAngularStiffness;
			CachedPhysicsHandle->InterpolationSpeed = 100.0f;
		}
		else
		{
			CachedPhysicsHandle->LinearDamping = FreeGrabLinearDamping;
			CachedPhysicsHandle->LinearStiffness = FreeGrabLinearStiffness;
			CachedPhysicsHandle->AngularDamping = FreeGrabAngularDamping;
			CachedPhysicsHandle->AngularStiffness = FreeGrabAngularStiffness;
			CachedPhysicsHandle->InterpolationSpeed = 50.0f;
		}

		// 执行抓取
		CachedPhysicsHandle->GrabComponentAtLocationWithRotation(
			Primitive,
			GrabBoneName,
			GrabLocation,
			GrabRotation
		);
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
	case EGrabType::WeaponSnap:
	case EGrabType::HumanBody:
		ReleasePhysicsHandle();
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
	GrabbedBoneName = NAME_None;

	// 广播委托
	OnObjectReleased.Broadcast(ReleasedActor);
}

void UPlayerGrabHand::SetPhysicsHandle(UPhysicsHandleComponent* InPhysicsHandle)
{
	CachedPhysicsHandle = InPhysicsHandle;
}

void UPlayerGrabHand::SetInventory(UInventoryComponent* InInventory)
{
	CachedInventory = InInventory;
}

// ==================== 内部实现 ====================


void UPlayerGrabHand::ReleasePhysicsHandle()
{
	if (CachedPhysicsHandle && CachedPhysicsHandle->GrabbedComponent)
	{
		CachedPhysicsHandle->ReleaseComponent();
	}
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

