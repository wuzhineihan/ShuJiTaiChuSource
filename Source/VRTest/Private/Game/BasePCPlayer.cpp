// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/Characters/BasePCPlayer.h"
#include "Grabber/PCGrabHand.h"
#include "Game/InventoryComponent.h"
#include "Grabber/IGrabbable.h"
#include "Grabbee/GrabbeeObject.h"
#include "Grabbee/Bow.h"
#include "Grabbee/Arrow.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "Skill/PlayerSkillComponent.h"
#include "Skill/Stasis/StasisPoint.h"
#include "Game/CollisionConfig.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Game/PCClimbLadderComponent.h"

ABasePCPlayer::ABasePCPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	// 鍒涘缓绗竴浜虹О鎽勫儚鏈?
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(RootComponent);
	FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
	FirstPersonCamera->bUsePawnControlRotation = true;
	PlayerCamera = FirstPersonCamera;

	// Camera collision (probe)
	CameraCollision = CreateDefaultSubobject<USphereComponent>(TEXT("CameraCollision"));
	CameraCollision->SetupAttachment(FirstPersonCamera);
	CameraCollision->InitSphereRadius(CameraCollisionRadius);
	CameraCollision->SetCollisionProfileName(CP_PLAYER_CAMERA_COLLISION);
	CameraCollision->SetGenerateOverlapEvents(true);
	CameraCollision->SetCanEverAffectNavigation(false);

	PCClimbLadderComponent = CreateDefaultSubobject<UPCClimbLadderComponent>(TEXT("PCClimbLadderComponent"));

	// 鍒涘缓宸︽墜
	PCLeftHand = CreateDefaultSubobject<UPCGrabHand>(TEXT("LeftHand"));
	PCLeftHand->SetupAttachment(FirstPersonCamera);
	PCLeftHand->bIsRightHand = false;
	LeftHand = PCLeftHand;  // 璧嬪€肩�?BasePlayer 鐨勫熀绫绘寚閽?

	// 鍒涘缓宸︽墜纰版挒浣?
	LeftHandCollision = CreateDefaultSubobject<USphereComponent>(TEXT("LeftHandCollision"));
	LeftHandCollision->SetupAttachment(PCLeftHand);
	LeftHandCollision->SetSphereRadius(5.0f);
	LeftHandCollision->SetCollisionProfileName(CP_PLAYER_HAND);
	LeftHandCollision->SetGenerateOverlapEvents(true);
	PCLeftHand->HandCollision = LeftHandCollision;

	// 鍒涘缓鍙虫墜
	PCRightHand = CreateDefaultSubobject<UPCGrabHand>(TEXT("RightHand"));
	PCRightHand->SetupAttachment(FirstPersonCamera);
	PCRightHand->bIsRightHand = true;
	RightHand = PCRightHand;  // 璧嬪€肩�?BasePlayer 鐨勫熀绫绘寚閽?

	// 鍒涘缓鍙虫墜纰版挒浣?
	RightHandCollision = CreateDefaultSubobject<USphereComponent>(TEXT("RightHandCollision"));
	RightHandCollision->SetupAttachment(PCRightHand);
	RightHandCollision->SetSphereRadius(5.0f);
	RightHandCollision->SetCollisionProfileName(CP_PLAYER_HAND);
	RightHandCollision->SetGenerateOverlapEvents(true);
	PCRightHand->HandCollision = RightHandCollision;

	// 璁剧疆鍙屾墜寮曠�?
	PCLeftHand->OtherHand = PCRightHand;
	PCRightHand->OtherHand = PCLeftHand;

	if (UCharacterMovementComponent* CharMove = GetCharacterMovement())
	{
		CharMove->GetNavAgentPropertiesRef().bCanCrouch = true;
		CharMove->SetCrouchedHalfHeight(PCCrouchedHalfHeight);
		CharMove->MaxWalkSpeedCrouched = PCMaxCrouchWalkSpeed;
		CharMove->bCanWalkOffLedgesWhenCrouching = false;
		CharMove->BrakingDecelerationFlying = 5000;
	}
}

void ABasePCPlayer::BeginPlay()
{
	Super::BeginPlay();

	// 缁戝畾鎵嬬殑鎶撳�?閲婃斁濮旀墭锛岀敤浜庡悓姝ョ洰鏍囨娴嬬姸鎬?
	if (PCLeftHand)
	{
		PCLeftHand->OnObjectGrabbed.AddDynamic(this, &ABasePCPlayer::OnHandGrabbedObject);
	}
	if (PCRightHand)
	{
		PCRightHand->OnObjectGrabbed.AddDynamic(this, &ABasePCPlayer::OnHandGrabbedObject);
	}
	
	if (FirstPersonCamera)
		RegularCameraRelativeZ = FirstPersonCamera->GetRelativeLocation().Z;
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
		RegularCapsuleHalfHeight = Capsule->GetUnscaledCapsuleHalfHeight();
	bIsCrouchCameraInterping = false;
}

void ABasePCPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	// 杈撳叆缁戝畾鍦ㄨ摑鍥句腑閰嶇疆锛圗nhanced Input�?
}

void ABasePCPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateTargetDetection();
	
	if (bIsCrouchCameraInterping)
		UpdateCrouchCameraInterp(DeltaTime);
}

// ==================== 閲嶅啓鍩虹被 ====================

void ABasePCPlayer::SetBowArmed(bool bArmed)
{
	if (bIsBowArmed && !bArmed)
	{
		if (bIsDrawingBow)
		{
			CancelDrawBow();
		}

		if (bIsAiming)
		{
			StopAiming();
		}

		CleanupPreparedArrowWhenExitBowMode();
	}

	Super::SetBowArmed(bArmed);

	if (bArmed)
	{
		EnsurePreparedArrowInRightHand();
	}
}

void ABasePCPlayer::HandleLeftTrigger(bool bPressed)
{
	if (!bIsBowArmed)
	{
		// ͽ��ģʽ
		if (bPressed)
		{
			// ���ƻ��⣺PC ����ʱ����˫��ץȡ
			if (PlayerSkillComponent && PlayerSkillComponent->IsDrawing())
			{
				return;
			}

			PCLeftHand->TryGrabOrRelease();
		}
	}
	else
	{
		// Bow mode: left trigger controls draw/release (fire)
		if (bIsAiming)
		{
			if (bPressed)
			{
				StartDrawBow();
			}
			else
			{
				ReleaseBowString();
			}
		}
	}
}
void ABasePCPlayer::HandleRightTrigger(bool bPressed)
{
	if (!bIsBowArmed)
	{
		// ͽ��ģʽ
		if (bPressed)
		{
			// ���ƻ��⣺PC ����ʱ����˫��ץȡ
			if (PlayerSkillComponent && PlayerSkillComponent->IsDrawing())
			{
				return;
			}

			PCRightHand->TryGrabOrRelease();
		}
	}
	else
	{
		// Bow mode: right trigger controls aiming on/off
		if (bPressed)
		{
			StartAiming();
		}
		else
		{
			StopAiming();
		}
	}
}
void ABasePCPlayer::HandleMoveInput(FVector2D MoveInput)
{
	if (PCClimbLadderComponent)
	{
		const FRotator Rotation = GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		PCClimbLadderComponent->HandleMoveInput(ForwardDirection, RightDirection, MoveInput);
	}
}

void ABasePCPlayer::StartStarDraw()
{
	if (PCLeftHand->bIsHolding && PCRightHand->bIsHolding)
		return;
	
	bool bIsRightHandFree = !PCRightHand->bIsHolding;
	
	if (PlayerSkillComponent)
		PlayerSkillComponent->StartStarDraw(FirstPersonCamera, bIsRightHandFree);
}

void ABasePCPlayer::StopStarDraw()
{
	PlayerSkillComponent ->FinishStarDraw();
}

void ABasePCPlayer::IgniteBySight()
{
	if (!bCanIgniteBySight || !bIsBowArmed)
	{
		return;
	}

	AArrow* ArrowToIgnite = nullptr;
	if (CurrentBow && CurrentBow->NockedArrow)
	{
		ArrowToIgnite = CurrentBow->NockedArrow;
	}
	else
	{
		ArrowToIgnite = GetHeldRightHandArrow();
	}

	if (ArrowToIgnite)
	{
		ArrowToIgnite->CatchFire();
	}
}

void ABasePCPlayer::SetCrouched(bool bCrouch)
{
	UCharacterMovementComponent* CharMove = GetCharacterMovement();
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (!CharMove || !Capsule || !FirstPersonCamera)
	{
		return;
	}

	const bool bWasCrouching = CharMove->IsCrouching();

	if (bCrouch)
	{
		if (bWasCrouching)
		{
			return;
		}

		float HalfHeightBefore = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
		float HalfHeightAfter = CharMove->GetCrouchedHalfHeight();
		FirstPersonCamera->AddRelativeLocation(FVector(0,0,HalfHeightBefore - HalfHeightAfter));
		bIsCrouchCameraInterping = true;
		Crouch();
	}
	else
	{
		if (!bWasCrouching)
		{
			return;
		}
		
		float HalfHeightBefore = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
		float HalfHeightAfter = RegularCapsuleHalfHeight;
		FirstPersonCamera->AddRelativeLocation(FVector(0,0,HalfHeightBefore - HalfHeightAfter));
		bIsCrouchCameraInterping = true;
		UnCrouch();
	}
}

void ABasePCPlayer::TryThrow(bool bRightHand)
{
	if (bIsBowArmed)
		return;
	
	UPCGrabHand* ThrowHand = bRightHand ? PCRightHand : PCLeftHand;
	if (!ThrowHand || !FirstPersonCamera)
	{
		return;
	}

	// 鎵嬮噷娌′笢瑗垮氨杩斿洖
	if (!ThrowHand->bIsHolding || !ThrowHand->HeldActor)
	{
		return;
	}

	// 鐗规畩澶勭悊锛歋tasisPoint 鎶曟�?
	if (AStasisPoint* StasisPoint = Cast<AStasisPoint>(ThrowHand->HeldActor))
	{
		HandleStasisPointThrow(ThrowHand, StasisPoint);
		return;
	}

	// 鍙�?GrabbeeObject 鎵嶅厑璁告姇�?
	AGrabbeeObject* ThrowObject = Cast<AGrabbeeObject>(ThrowHand->HeldActor);
	if (!ThrowObject)
	{
		return;
	}

	// 閫氳繃灏勭嚎璁＄畻鎶曟幏鐩爣鐐癸紙浠庢憚鍍忔満鏈濆墠锛?
	FHitResult Hit;
	const bool bHit = PerformLineTrace(Hit, MaxThrowDistance, TCC_PROJECTILE);

	const FVector Start = FirstPersonCamera->GetComponentLocation();
	const FVector End = Start + FirstPersonCamera->GetForwardVector() * MaxThrowDistance;
	const FVector TargetPoint = bHit ? Hit.ImpactPoint : End;

	// 鍏堥噴鏀撅紙瑙ｉ�?PhysicsHandle / 闄勭潃锛夛紝鍐嶅彂灏?
	ThrowHand->ReleaseObject();

	// LaunchTowards 鍐呴儴浼氭竻閫熷害骞跺姞鍐查�?
	bool bSuccess = ThrowObject->LaunchTowards(TargetPoint, ThrowArcParam);
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("ABasePCPlayer::TryThrow: LaunchTowards failed!"));
	}
}

void ABasePCPlayer::HandleStasisPointThrow(UPCGrabHand* ThrowHand, AStasisPoint* StasisPoint)
{
	if (!ThrowHand || !StasisPoint || !FirstPersonCamera)
	{
		return;
	}

	// 1) 璁＄畻鍙戝皠涓婁笅鏂囷紙PC锛氬熀浜庣浉鏈哄墠鍚戯�?
	const FVector CameraLocation = FirstPersonCamera->GetComponentLocation();
	const FVector CameraForward = FirstPersonCamera->GetForwardVector();

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);
	IgnoreActors.Add(StasisPoint);
	// 棰濆鐨勫拷鐣ュ璞★紙姣斿鍙屾墜鎵嬫寔鐗╋級�?StasisPoint 鍐呴儴缁撳悎 HoldingHand 澶勭悊锟斤拷
	// 杩欓噷浠嶄繚鐣欒皟鐢ㄧ鍙紶鍏ョ殑 IgnoreActors 鎵╁睍鑳藉姏�?

	// 2) 璁＄畻鍒濋€熷害
	const FVector InitVelocity = CameraForward * StasisFireSpeedScalar;

	// 3) 閲婃斁锛堣В闄ゆ姄鍙栵�?
	ThrowHand->ReleaseObject();

	// 4) 鍙戝皠锛氱敱瀹氳韩鐞冨唴閮ㄨ嚜琛屾壘鐩爣锛屾壘涓嶅埌鍒欑洿椋炲苟瓒呮椂鑷�?
	StasisPoint->Fire(
		this,
		CameraLocation,
		CameraForward,
		InitVelocity,
		StasisDetectionRadius,
		StasisDetectionAngle,
		IgnoreActors
	);

	// 5) 瑙ｉ攣鎵嬮儴
	ThrowHand->SetGrabLock(false);
}

// ==================== 寮撶鎿嶄綔 ====================

void ABasePCPlayer::StartAiming()
{
	if (!bIsBowArmed || !bHasBow)
	{
		return;
	}

	bIsAiming = true;
	PCLeftHand->InterpToTransform(AimingLeftHandTransform);

	if (EnsurePreparedArrowInRightHand())
	{
		NockPreparedArrowFromRightHand();
	}
}

void ABasePCPlayer::StopAiming()
{
	if (bIsDrawingBow)
	{
		CancelDrawBow();
	}

	bIsAiming = false;
	PCLeftHand->InterpToDefaultTransform();

	UnnockArrowToRightHand();
}

void ABasePCPlayer::StartDrawBow()
{
	if (!bIsAiming || !CurrentBow || bIsDrawingBow)
	{
		return;
	}

	if (!CurrentBow->NockedArrow)
	{
		if (!EnsurePreparedArrowInRightHand() || !NockPreparedArrowFromRightHand())
		{
			PlayNoArrowSound();
			return;
		}
	}

	bIsDrawingBow = true;

	FVector StringRestPos = CurrentBow->StringRestPosition ?
		CurrentBow->StringRestPosition->GetComponentLocation() :
		CurrentBow->StringMesh->GetComponentLocation();

	FTransform StringTransform;
	StringTransform.SetLocation(StringRestPos);
	StringTransform.SetRotation(CurrentBow->GetActorRotation().Quaternion());
	PCRightHand->SetWorldTransform(StringTransform);

	CurrentBow->bStringHeld = true;
	CurrentBow->StringHoldingHand = PCRightHand;
	CurrentBow->InitialStringGrabOffset = StringRestPos - PCRightHand->GetComponentLocation();
	if (CurrentBow->ArrowTracePreview)
	{
		CurrentBow->ArrowTracePreview->SetVisibility(CurrentBow->NockedArrow != nullptr);
	}

	if (FirstPersonCamera && PCRightHand)
	{
		const FVector PullDirWorld = -FirstPersonCamera->GetForwardVector().GetSafeNormal();
		const FVector RightHandTargetWorld = PCRightHand->GetComponentLocation() + PullDirWorld * PCDrawDistance;

		FTransform RightHandTargetRelative;
		RightHandTargetRelative.SetLocation(FirstPersonCamera->GetComponentTransform().InverseTransformPosition(RightHandTargetWorld));
		RightHandTargetRelative.SetRotation(PCRightHand->GetComponentRotation().Quaternion());
		RightHandTargetRelative.SetScale3D(FVector::OneVector);

		PCRightHand->InterpToTransform(RightHandTargetRelative);
	}
}

void ABasePCPlayer::CancelDrawBow()
{
	if (!bIsDrawingBow)
	{
		return;
	}

	bIsDrawingBow = false;

	ReleasePCStringHoldWithoutFiring();

	if (CurrentBow)
	{
		const FVector RestPos = CurrentBow->StringRestPosition ? CurrentBow->StringRestPosition->GetComponentLocation() : CurrentBow->StringMesh->GetComponentLocation();
		CurrentBow->CurrentGrabSpot = RestPos;
		CurrentBow->CurrentPullLength = 0.0f;
		CurrentBow->StringVelocity = FVector::ZeroVector;
		CurrentBow->bPlayedTightSound = false;

		if (CurrentBow->StringMID)
		{
			CurrentBow->StringMID->SetVectorParameterValue(FName(TEXT("GrabSpot")), FLinearColor(RestPos));
		}
		if (CurrentBow->ArrowTracePreview)
		{
			CurrentBow->ArrowTracePreview->SetVisibility(false);
		}
	}

	if (PCRightHand)
	{
		PCRightHand->InterpToDefaultTransform();
	}
}
void ABasePCPlayer::StopDrawBow()
{
	// DEPRECATED: 涓€鏃﹀紑濮嬫媺寮撳氨涓嶈兘鍙栨秷锛屾澗鎵嬫垨鍒囨崲妯″紡閮戒細鐩存帴鍙戝�?
	// 姝ゅ嚱鏁颁繚鐣欑敤浜庡吋瀹癸紝浣嗗唴閮ㄧ洿鎺ヨ皟�?ReleaseBowString
	if (bIsDrawingBow)
	{
		CancelDrawBow();
	}
}

void ABasePCPlayer::ReleaseBowString()
{
	if (!bIsDrawingBow)
	{
		return;
	}

	bIsDrawingBow = false;

	if (CurrentBow)
	{
		CurrentBow->ReleaseString();
	}

	if (PCRightHand)
	{
		PCRightHand->InterpToDefaultTransform();
	}
}

// ==================== 鍐呴儴鍑芥暟 ====================

void ABasePCPlayer::UpdateTargetDetection()
{
	FHitResult Hit;
	AActor* NewTarget = nullptr;
	FName NewBoneName = NAME_None;
	FVector NewImpactPoint = FVector::ZeroVector;

	bTraceHit = PerformLineTrace(Hit, MaxGrabDistance, GrabTraceChannel);

	bool bSightHitsIgniteTarget = false;
	bCanIgniteBySight = false;
	IgniteBySightImpactPoint = FVector::ZeroVector;
	if (bTraceHit)
	{
		if (UPrimitiveComponent* HitComp = Hit.GetComponent())
		{
			if (!IgniteBySightComponentTag.IsNone() && HitComp->ComponentHasTag(IgniteBySightComponentTag))
			{
				bSightHitsIgniteTarget = true;
				IgniteBySightImpactPoint = Hit.ImpactPoint;
			}
		}
	}

	const bool bHasPreparedArrow = (CurrentBow && CurrentBow->NockedArrow != nullptr) || (GetHeldRightHandArrow() != nullptr);
	bCanIgniteBySight = bIsBowArmed && bSightHitsIgniteTarget && bHasPreparedArrow;

	if (bIsBowArmed)
	{
		if (TargetedObject && IsValid(TargetedObject))
		{
			if (Cast<IGrabbable>(TargetedObject))
			{
				IGrabbable::Execute_OnGrabDeselected(TargetedObject);
			}
		}
		TargetedObject = nullptr;
		TargetedBoneName = NAME_None;
		TargetedImpactPoint = FVector::ZeroVector;
		return;
	}

	if (bTraceHit)
	{
		AActor* HitActor = Hit.GetActor();
		IGrabbable* Grabbable = Cast<IGrabbable>(HitActor);

		if (Grabbable)
		{
			UPCGrabHand* CheckHand = !PCLeftHand->bIsHolding ? PCLeftHand : PCRightHand;
			if (IGrabbable::Execute_CanBeGrabbedBy(HitActor, CheckHand))
			{
				NewTarget = HitActor;
				NewBoneName = Hit.BoneName;
				NewImpactPoint = Hit.ImpactPoint;
			}
		}
	}

	if (NewTarget != TargetedObject)
	{
		AActor* OldTarget = TargetedObject;
		TargetedObject = NewTarget;
		TargetedBoneName = NewBoneName;
		TargetedImpactPoint = NewImpactPoint;

		if (OldTarget && IsValid(OldTarget))
		{
			if (Cast<IGrabbable>(OldTarget))
			{
				IGrabbable::Execute_OnGrabDeselected(OldTarget);
			}
		}
		if (NewTarget && IsValid(NewTarget))
		{
			if (Cast<IGrabbable>(NewTarget))
			{
				IGrabbable::Execute_OnGrabSelected(NewTarget);
			}
		}
	}
	else
	{
		TargetedBoneName = NewBoneName;
		TargetedImpactPoint = NewImpactPoint;
	}
}

bool ABasePCPlayer::PerformLineTrace(FHitResult& OutHit, float MaxDistance, ECollisionChannel TraceChannel) const
{
	if (!FirstPersonCamera)
	{
		return false;
	}

	FVector Start = FirstPersonCamera->GetComponentLocation();
	FVector End = Start + FirstPersonCamera->GetForwardVector() * MaxDistance;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, TraceChannel, QueryParams);
	if (bDrawGrabLineTraceDebug)
	{
		const float LifeTime = GrabLineTraceDebugDrawTime;
		const FVector HitPoint = bHit ? OutHit.ImpactPoint : End;

		DrawDebugLine(GetWorld(), Start, HitPoint, bHit ? FColor::Green : FColor::Red, false, LifeTime, 0, GrabLineTraceDebugThickness);

		if (bHit)
		{
			DrawDebugPoint(GetWorld(), OutHit.ImpactPoint, 10.0f, FColor::Yellow, false, LifeTime);
		}
	}
	return bHit;
}

void ABasePCPlayer::OnHandGrabbedObject(AActor* GrabbedObject)
{
	// 褰撲换涓€鍙墜鎶撳彇鐗╀綋鏃讹紝绔嬪嵆娓呯┖鐬勫噯鐩爣
	if (TargetedObject && IsValid(TargetedObject))
	{
		AActor* OldTarget = TargetedObject;
		TargetedObject = nullptr;
		TargetedBoneName = NAME_None;
		TargetedImpactPoint = FVector::ZeroVector;

		// 鍙栨秷閫変腑鐘舵€侊紙閫氳繃鎺ュ彛锛?
		if (Cast<IGrabbable>(OldTarget))
		{
			IGrabbable::Execute_OnGrabDeselected(OldTarget);
		}
	}
}


AArrow* ABasePCPlayer::GetHeldRightHandArrow() const
{
	if (!PCRightHand || !PCRightHand->bIsHolding)
	{
		return nullptr;
	}
	return Cast<AArrow>(PCRightHand->HeldActor);
}

bool ABasePCPlayer::EnsurePreparedArrowInRightHand()
{
	if (!bIsBowArmed || !PCRightHand)
	{
		return false;
	}

	if (CurrentBow && CurrentBow->NockedArrow)
	{
		return true;
	}

	if (GetHeldRightHandArrow())
	{
		return true;
	}

	if (!InventoryComponent || !InventoryComponent->HasArrow())
	{
		return false;
	}

	const FTransform SpawnTransform = PCRightHand->GetComponentTransform();
	AGrabbeeObject* Spawned = InventoryComponent->TryRetrieveArrow(SpawnTransform);
	AArrow* Arrow = Cast<AArrow>(Spawned);
	if (!Arrow)
	{
		if (Spawned)
		{
			Spawned->Destroy();
		}
		return false;
	}

	PCRightHand->GrabObject(Arrow);
	return GetHeldRightHandArrow() == Arrow;
}

bool ABasePCPlayer::NockPreparedArrowFromRightHand()
{
	if (!CurrentBow || !PCRightHand)
	{
		return false;
	}

	if (CurrentBow->NockedArrow)
	{
		return true;
	}

	AArrow* HeldArrow = GetHeldRightHandArrow();
	if (!HeldArrow)
	{
		return false;
	}

	PCRightHand->ReleaseObject();
	if (!CurrentBow->NockArrow(HeldArrow))
	{
		PCRightHand->GrabObject(HeldArrow);
		return false;
	}

	return CurrentBow->NockedArrow == HeldArrow;
}

bool ABasePCPlayer::UnnockArrowToRightHand()
{
	if (!CurrentBow || !PCRightHand)
	{
		return false;
	}

	ReleasePCStringHoldWithoutFiring();

	if (!CurrentBow->NockedArrow)
	{
		const FVector RestPos = CurrentBow->StringRestPosition ? CurrentBow->StringRestPosition->GetComponentLocation() : CurrentBow->StringMesh->GetComponentLocation();
		CurrentBow->CurrentGrabSpot = RestPos;
		CurrentBow->CurrentPullLength = 0.0f;
		CurrentBow->StringVelocity = FVector::ZeroVector;
		CurrentBow->bPlayedTightSound = false;
		if (CurrentBow->StringMID)
		{
			CurrentBow->StringMID->SetVectorParameterValue(FName(TEXT("GrabSpot")), FLinearColor(RestPos));
		}
		if (CurrentBow->ArrowTracePreview)
		{
			CurrentBow->ArrowTracePreview->SetVisibility(false);
		}
		return true;
	}

	AArrow* NockedArrow = CurrentBow->NockedArrow;
	CurrentBow->UnnockArrow();

	const FVector RestPos = CurrentBow->StringRestPosition ? CurrentBow->StringRestPosition->GetComponentLocation() : CurrentBow->StringMesh->GetComponentLocation();
	CurrentBow->CurrentGrabSpot = RestPos;
	CurrentBow->CurrentPullLength = 0.0f;
	CurrentBow->StringVelocity = FVector::ZeroVector;
	CurrentBow->bPlayedTightSound = false;
	if (CurrentBow->StringMID)
	{
		CurrentBow->StringMID->SetVectorParameterValue(FName(TEXT("GrabSpot")), FLinearColor(RestPos));
	}
	if (CurrentBow->ArrowTracePreview)
	{
		CurrentBow->ArrowTracePreview->SetVisibility(false);
	}

	PCRightHand->GrabObject(NockedArrow);
	return GetHeldRightHandArrow() == NockedArrow;
}
void ABasePCPlayer::ReleasePCStringHoldWithoutFiring()
{
	if (!CurrentBow || !PCRightHand)
	{
		return;
	}

	if (CurrentBow->bStringHeld && CurrentBow->StringHoldingHand == PCRightHand)
	{
		CurrentBow->bStringHeld = false;
		CurrentBow->StringHoldingHand = nullptr;
	}

	if (CurrentBow->InStringCollisionHand == PCRightHand)
	{
		CurrentBow->InStringCollisionHand = nullptr;
	}

	if (PCRightHand->HeldActor == CurrentBow)
	{
		PCRightHand->HeldActor = nullptr;
		PCRightHand->HeldGrabType = EGrabType::None;
		PCRightHand->bIsHolding = false;
		PCRightHand->GrabbedBoneName = NAME_None;
	}
}
void ABasePCPlayer::CleanupPreparedArrowWhenExitBowMode()
{
	if (CurrentBow && CurrentBow->NockedArrow)
	{
		if (!UnnockArrowToRightHand())
		{
			AArrow* NockedArrow = CurrentBow->NockedArrow;
			CurrentBow->UnnockArrow();
			StoreAndDestroyArrow(NockedArrow);
		}
	}

	if (AArrow* HeldArrow = GetHeldRightHandArrow())
	{
		StoreAndDestroyArrow(HeldArrow);
	}
}

void ABasePCPlayer::StoreAndDestroyArrow(AArrow* Arrow)
{
	if (!Arrow)
	{
		return;
	}

	if (PCRightHand && PCRightHand->HeldActor == Arrow)
	{
		PCRightHand->ReleaseObject();
	}

	if (InventoryComponent)
	{
		InventoryComponent->TryStoreArrow();
	}

	Arrow->Destroy();
}
void ABasePCPlayer::PlayNoArrowSound()
{
	// TODO: 
}

void ABasePCPlayer::UpdateCrouchCameraInterp(float DeltaTime)
{
	if (!FirstPersonCamera)
	{
		return;
	}

	FVector Rel = FirstPersonCamera->GetRelativeLocation();
	Rel.Z = FMath::FInterpTo(Rel.Z, RegularCameraRelativeZ, DeltaTime, PCCrouchCameraInterpSpeed);

	const float Dist = FMath::Abs(Rel.Z - RegularCameraRelativeZ);
	if (Dist <= PCCrouchCameraStopThreshold)
	{
		Rel.Z = RegularCameraRelativeZ;
		bIsCrouchCameraInterping = false;
	}

	FirstPersonCamera->SetRelativeLocation(Rel);
}


