// Fill out your copyright notice in the Description page of Project Settings.

#include "Grabber/PlayerGrabHand.h"
#include "Game/InventoryComponent.h"
#include "Grabber/IGrabbable.h"
#include "Grabbee/GrabbeeWeapon.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Game/Characters/BasePlayer.h"
#include "Game/Characters/BasePCPlayer.h"
#include "Audio/AudioSubsystem.h"
#include "Game/CollisionConfig.h"

UPlayerGrabHand::UPlayerGrabHand()
{
	PrimaryComponentTick.bCanEverTick = true;

	// 榛樿妫€娴嬪璞＄被鍨?
	GrabObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	GrabObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody));
	
	// HandCollision 鐢?BaseVRPlayer 鍒涘缓骞惰祴鍊?
}

void UPlayerGrabHand::BeginPlay()
{
	Super::BeginPlay();

	CachedAudioSubsystem = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			CachedAudioSubsystem = GI->GetSubsystem<UAudioSubsystem>();
		}
	}
	
	PlayerCharacter = Cast<ABasePlayer>(GetOwner());
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerGrabHand: Owner is not ABasePlayer"));
		return;
	}
	// CachedPhysicsHandle 鍜?CachedInventory 灏嗙敱 BasePlayer 鍦ㄥ叾 BeginPlay 涓缃?
}

void UPlayerGrabHand::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 纭繚閲婃斁 PhysicsHandle
	ReleasePhysicsHandle();

	// 娓呯┖缂撳瓨鐨勭粍浠跺紩鐢?
	CachedPhysicsHandle = nullptr;
	CachedInventory = nullptr;

	Super::EndPlay(EndPlayReason);
}

void UPlayerGrabHand::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 鏇存柊 PhysicsHandle 鐩爣浣嶇疆锛堝鏋滄鍦ㄦ姄鍙栵級
	if (bIsHolding && HeldActor && CachedPhysicsHandle && CachedPhysicsHandle->GrabbedComponent)
	{
		// 鏍规嵁缂撳瓨鐨勬姄鍙栫被鍨嬭缃洰鏍?
		switch (HeldGrabType)
		{
		case EGrabType::Free:
		case EGrabType::WeaponSnap:
			{
				// 灏嗗眬閮ㄥ亸绉昏浆鎹负涓栫晫绌洪棿
				FTransform CurrentHandTransform = GetComponentTransform();
				FVector TargetPosition = CurrentHandTransform.TransformPosition(GrabOffset.GetLocation());
				FRotator TargetRotation = (CurrentHandTransform.GetRotation() * GrabOffset.GetRotation()).Rotator();
				
				CachedPhysicsHandle->SetTargetLocationAndRotation(TargetPosition, TargetRotation);
			}
			break;
		case EGrabType::HumanBody:
			// HumanBody: 鐩存帴璺熼殢鎵嬮儴浣嶇疆
			CachedPhysicsHandle->SetTargetLocationAndRotation(GetComponentLocation(), GetComponentRotation());
			break;
		default:
			break;
		}
	}
}

// ==================== 鏍稿績鎺ュ彛 ====================

void UPlayerGrabHand::TryGrab(bool bFromBackpack)
{
	// Step 0: 妫€鏌ユ姄鍙栭攣
	if (bGrabLocked)
	{
		return;
	}

	// Step 1: 鏈夋晥鎬ф楠?
	if (bIsHolding)
	{
		return;
	}

	// Step 2: 鏌ユ壘鎶撳彇鐩爣
	FName BoneName = NAME_None;
	AActor* TargetActor = FindTarget(bFromBackpack, BoneName);
	
	if (!TargetActor)
	{
		return;
	}

	// Step 4: 鎶撳彇锛岀粺涓€鍦℅rabObject閲岃繘琛岄獙璇?
	GrabObject(TargetActor, BoneName);
}

void UPlayerGrabHand::TryRelease(bool bToBackpack)
{
	// Step 0: 妫€鏌ユ姄鍙栭攣
	if (bGrabLocked)
	{
		return;
	}

	// ValidateRelease 鍐呴儴澶勭悊鎵€鏈夋湁鏁堟€ф鏌?
	if (!(bIsHolding && HeldActor != nullptr))
	{
		return;
	}

	if (bToBackpack)
	{
		// 鍙湁绠彲浠ユ斁鍏ヨ儗鍖?
		if (AGrabbeeWeapon* Weapon = Cast<AGrabbeeWeapon>(HeldActor))
		{
			if (Weapon->WeaponType == EWeaponType::Arrow)
			{
				if (CachedInventory && CachedInventory->TryStoreArrow())
				{
					// 淇濆瓨鎸囬拡鐢ㄤ簬閿€姣?
					AActor* ArrowToDestroy = HeldActor;
					
					// 缁熶竴閫氳繃 ReleaseObject 閲婃斁锛堝鐞嗙墿鐞嗘帶鍒躲€佺姸鎬佹竻鐞嗐€佸洖璋冿級
					ReleaseObject();
					
					// 閿€姣佺 Actor
					ArrowToDestroy->Destroy();
					return;
				}
			}
		}
	}

	// 姝ｅ父閲婃斁
	ReleaseObject();
}

AActor* UPlayerGrabHand::FindTarget(bool bFromBackpack, FName& OutBoneName)
{
	OutBoneName = NAME_None;
	
	// 浼樺厛浠庤儗鍖呭彇鐗?
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

	// 鍩虹被涓嶅疄鐜扮墿鐞嗘娴嬶紝鐢卞瓙绫婚噸鍐?
	return nullptr;
}


// ==================== 鎶撳彇瀹炵幇 ====================

void UPlayerGrabHand::GrabObject(AActor* TargetActor, FName BoneName)
{
	if (!TargetActor || !CachedPhysicsHandle)
	{
		return;
	}

	// Keep held-state transitions consistent when switching objects on the same hand.
	if (bIsHolding)
	{
		if (HeldActor == TargetActor)
		{
			return;
		}

		ReleaseObject();
		if (bIsHolding)
		{
			return;
		}
	}
	
	// 鑾峰彇鎺ュ彛
	IGrabbable* Grabbable = Cast<IGrabbable>(TargetActor);
	if (!Grabbable)
	{
		UE_LOG(LogTemp, Warning, TEXT("GrabObject: Target does not implement IGrabbable"));
		return;
	}

	// 鍦?GrabObject 寮€澶寸粺涓€妫€鏌?CanBeGrabbedBy
	if (!IGrabbable::Execute_CanBeGrabbedBy(TargetActor, this))
	{
		UE_LOG(LogTemp, Warning, TEXT("GrabObject: Target cannot be grabbed by this hand"));
		return;
	}

	// 鑾峰彇 Primitive锛堟墍鏈夌墿鐞嗘姄鍙栫被鍨嬮兘闇€瑕侊級
	UPrimitiveComponent* Primitive = IGrabbable::Execute_GetGrabPrimitive(TargetActor);
	if (!Primitive && IGrabbable::Execute_GetGrabType(TargetActor) != EGrabType::Custom)
	{
		UE_LOG(LogTemp, Warning, TEXT("GrabObject: Target's grab primitive is null"));
		return;
	}

	// 鑾峰彇鎶撳彇绫诲瀷
	EGrabType GrabType = IGrabbable::Execute_GetGrabType(TargetActor);

	// 澶勭悊鍙︿竴鍙墜鎸佹湁鐨勬儏鍐碉紙浠呭闈炲弻鎵嬫姄鍙栫墿浣擄級
	bool bSupportsDual = IGrabbable::Execute_SupportsDualHandGrab(TargetActor);
	if (!bSupportsDual)
	{
		HandleOtherHandHolding(TargetActor, Grabbable);
	}

	// 淇濆瓨楠ㄩ鍚嶅拰鎶撳彇绫诲瀷
	GrabbedBoneName = BoneName;
	HeldGrabType = GrabType;

	// ==================== 鏍规嵁鎶撳彇绫诲瀷澶勭悊鐗瑰畾閫昏緫 ====================
	FVector GrabLocation;
	FRotator GrabRotation;
	FName GrabBoneName = NAME_None;
	bool bUseSnapStrength = false;

	switch (GrabType)
	{
	case EGrabType::Free:
		{
			// 璁＄畻鐗╀綋鐩稿浜庢墜鐨勫眬閮ㄥ亸绉?
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
				// 閫氱煡鐜╁瑙掕壊棣栨鎷惧彇寮?
				if (PlayerCharacter && PlayerCharacter->CheckBowFirstPickedUp())
				{
					TargetActor->Destroy();
					return;
				}
			}
			
			// 鏌ユ壘姝ゆ鍣ㄧ被鍨嬬殑鍋忕Щ
			FTransform* Offset = WeaponGrabOffsets.Find(Weapon->WeaponType);
			GrabOffset = Offset ? *Offset : FTransform::Identity;

			// 鐬Щ姝﹀櫒鍒版墜閮ㄤ綅缃?
			Primitive->SetSimulatePhysics(false);
			
			FTransform HandTransform = GetComponentTransform();
			FVector TargetPosition = HandTransform.TransformPosition(GrabOffset.GetLocation());
			FRotator TargetRotation = (HandTransform.GetRotation() * GrabOffset.GetRotation()).Rotator();
			
			Weapon->SetActorLocationAndRotation(TargetPosition, TargetRotation);
			Primitive->SetSimulatePhysics(true);
			

			// GrabLocation 鏄墿浣撲笂鐨勬姄鍙栫偣锛堣川蹇冿級锛屼笉鏄洰鏍囦綅缃?
			GrabLocation = Primitive->GetComponentLocation();
			GrabRotation = Primitive->GetComponentRotation();

			bUseSnapStrength = true;
		}
		break;

	case EGrabType::HumanBody:
		{
			// 濡傛灉鏈夐楠煎悕鍒欎娇鐢ㄩ楠间綅缃?
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
		// Custom 绫诲瀷涓嶄娇鐢?PhysicsHandle锛岃烦杩囩墿鐞嗘姄鍙栭€昏緫
		break;

	default:
		return;
	}

	// ==================== 鍏卞悓閫昏緫锛氶厤缃?PhysicsHandle 骞舵墽琛屾姄鍙?====================
	if (GrabType != EGrabType::Custom)
	{
		// 閰嶇疆 PhysicsHandle 鍙傛暟
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

		// 鎵ц鎶撳彇
		CachedPhysicsHandle->GrabComponentAtLocationWithRotation(
			Primitive,
			GrabBoneName,
			GrabLocation,
			GrabRotation
		);
	}

	// 鏇存柊鐘舵€?
	HeldActor = TargetActor;
	bIsHolding = true;

	// PC only: keep physics valid; only ignore Pawn collision while held.
	HeldCollisionComponent.Reset();
	CachedHeldCollisionProfile = NAME_None;
		if (Cast<ABasePCPlayer>(PlayerCharacter))
	{
		HeldCollisionComponent = Primitive;
		if (Primitive)
		{
			CachedHeldCollisionProfile = Primitive->GetCollisionProfileName();
			Primitive->SetCollisionProfileName(CP_PC_HELD_OBJECT);
		}
	}

	IGrabbable::Execute_OnGrabbed(TargetActor, this);

	// 骞挎挱濮旀墭
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

	// 鏍规嵁鎶撳彇绫诲瀷鎵ц閲婃斁
	switch (GrabType)
	{
	case EGrabType::Free:
	case EGrabType::WeaponSnap:
	case EGrabType::HumanBody:
		ReleasePhysicsHandle();
		break;
	case EGrabType::Custom:
		// Custom 绫诲瀷涓嶅仛棰濆鐗╃悊澶勭悊
		break;
	default:
		break;
	}

	// 閫氱煡鐗╀綋琚噴鏀撅紙閫氳繃鎺ュ彛锛?
	if (ReleasedActor && ReleasedActor->GetClass()->ImplementsInterface(UGrabbable::StaticClass()))
	{
		IGrabbable::Execute_OnReleased(ReleasedActor, this);
	}

	// Restore collision settings for PC-held object.
	if (UPrimitiveComponent* HeldComp = HeldCollisionComponent.Get())
	{
		if (!CachedHeldCollisionProfile.IsNone())
		{
			HeldComp->SetCollisionProfileName(CachedHeldCollisionProfile);
		}
	}
	HeldCollisionComponent.Reset();
	CachedHeldCollisionProfile = NAME_None;
	
	// 鏇存柊鐘舵€?
	HeldActor = nullptr;
	HeldGrabType = EGrabType::None;
	bIsHolding = false;
	GrabbedBoneName = NAME_None;

	// 骞挎挱濮旀墭
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

void UPlayerGrabHand::SetGrabLock(bool bLock)
{
	bGrabLocked = bLock;
}

// ==================== 鍐呴儴瀹炵幇 ====================


void UPlayerGrabHand::ReleasePhysicsHandle()
{
	if (CachedPhysicsHandle && CachedPhysicsHandle->GrabbedComponent)
	{
		CachedPhysicsHandle->ReleaseComponent();
	}
}

// ==================== 杈呭姪鍑芥暟 ====================

void UPlayerGrabHand::HandleOtherHandHolding(AActor* TargetActor, IGrabbable* Grabbable)
{
	if (!OtherHand || !TargetActor)
	{
		return;
	}

	// 濡傛灉鍙︿竴鍙墜鎸佹湁鍚屼竴鐗╀綋锛屽厛閲婃斁
	if (OtherHand->HeldActor == TargetActor)
	{
		OtherHand->ReleaseObject();
	}
}

