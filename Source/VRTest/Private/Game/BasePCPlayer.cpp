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
#include "Game/PCWindowVaultComponent.h"
#include "UI/PCActionPromptComponent.h"

ABasePCPlayer::ABasePCPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	// 閸掓稑缂撶粭顑跨娴滆櫣袨閹藉嫬鍎氶張?
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

	PCWindowVaultComponent = CreateDefaultSubobject<UPCWindowVaultComponent>(TEXT("PCWindowVaultComponent"));

	PCActionPromptComponent = CreateDefaultSubobject<UPCActionPromptComponent>(TEXT("PCActionPromptComponent"));

	// 閸掓稑缂撳锔藉
	PCLeftHand = CreateDefaultSubobject<UPCGrabHand>(TEXT("LeftHand"));
	PCLeftHand->SetupAttachment(FirstPersonCamera);
	PCLeftHand->bIsRightHand = false;
	LeftHand = PCLeftHand;  // 鐠у鈧偐绮?BasePlayer 閻ㄥ嫬鐔€缁粯瀵氶柦?

	// 閸掓稑缂撳锔藉绾扮増鎸掓担?
	LeftHandCollision = CreateDefaultSubobject<USphereComponent>(TEXT("LeftHandCollision"));
	LeftHandCollision->SetupAttachment(PCLeftHand);
	LeftHandCollision->SetSphereRadius(5.0f);
	LeftHandCollision->SetCollisionProfileName(CP_PLAYER_HAND);
	LeftHandCollision->SetGenerateOverlapEvents(true);
	PCLeftHand->HandCollision = LeftHandCollision;

	// 閸掓稑缂撻崣铏
	PCRightHand = CreateDefaultSubobject<UPCGrabHand>(TEXT("RightHand"));
	PCRightHand->SetupAttachment(FirstPersonCamera);
	PCRightHand->bIsRightHand = true;
	RightHand = PCRightHand;  // 鐠у鈧偐绮?BasePlayer 閻ㄥ嫬鐔€缁粯瀵氶柦?

	// 閸掓稑缂撻崣铏绾扮増鎸掓担?
	RightHandCollision = CreateDefaultSubobject<USphereComponent>(TEXT("RightHandCollision"));
	RightHandCollision->SetupAttachment(PCRightHand);
	RightHandCollision->SetSphereRadius(5.0f);
	RightHandCollision->SetCollisionProfileName(CP_PLAYER_HAND);
	RightHandCollision->SetGenerateOverlapEvents(true);
	PCRightHand->HandCollision = RightHandCollision;

	// 鐠佸墽鐤嗛崣灞惧瀵洜鏁?
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

	// 缂佹垵鐣鹃幍瀣畱閹舵挸褰?闁插﹥鏂佹慨鏃€澧敍宀€鏁ゆ禍搴℃倱濮濄儳娲伴弽鍥梾濞村濮搁幀?
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
	
	// 鏉堟挸鍙嗙紒鎴濈暰閸︺劏鎽戦崶鍙ヨ厬闁板秶鐤嗛敍鍦梟hanced Input閿?
}

void ABasePCPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateTargetDetection();
	
	if (bIsCrouchCameraInterping)
		UpdateCrouchCameraInterp(DeltaTime);
}

// ==================== 闁插秴鍟撻崺铏硅 ====================

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
	if (bActionLocked)
	{
		return;
	}

	if (!bIsBowArmed)
	{
		// 徒手模式
		if (bPressed)
		{
			// 绘制互斥：PC 绘制时禁用双手抓取
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
	if (bActionLocked)
	{
		return;
	}

	if (!bIsBowArmed)
	{
		// 徒手模式
		if (bPressed)
		{
			// 绘制互斥：PC 绘制时禁用双手抓取
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
	if (bActionLocked)
	{
		return;
	}

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
	if (bActionLocked || !bStarDrawEnabled)
	{
		return;
	}

	if (PCLeftHand->bIsHolding && PCRightHand->bIsHolding)
		return;

	bool bIsRightHandFree = !PCRightHand->bIsHolding;

	if (PlayerSkillComponent)
		PlayerSkillComponent->StartStarDraw(FirstPersonCamera, bIsRightHandFree);
}

void ABasePCPlayer::StopStarDraw()
{
	if (bActionLocked || !bStarDrawEnabled)
	{
		return;
	}

	PlayerSkillComponent ->FinishStarDraw();
}


void ABasePCPlayer::IgniteBySight()
{
	if (bActionLocked)
	{
		return;
	}

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


void ABasePCPlayer::TryWindowVaultBySight()
{
	if (bActionLocked)
	{
		return;
	}

	if (!PCWindowVaultComponent)
	{
		return;
	}

	PCWindowVaultComponent->TryStartVaultBySight(TargetedHitComponent);
}

void ABasePCPlayer::SetActionLocked(bool bLocked)
{
	bActionLocked = bLocked;
}

void ABasePCPlayer::SetCrouched(bool bCrouch)
{
	if (bActionLocked)
	{
		return;
	}

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
	if (bActionLocked)
	{
		return;
	}

	if (bIsBowArmed)
		return;
	
	UPCGrabHand* ThrowHand = bRightHand ? PCRightHand : PCLeftHand;
	if (!ThrowHand || !FirstPersonCamera)
	{
		return;
	}

	// 閹靛鍣峰▽鈥茬鐟楀灝姘ㄦ潻鏂挎礀
	if (!ThrowHand->bIsHolding || !ThrowHand->HeldActor)
	{
		return;
	}

	// 閻楄鐣╂径鍕倞閿涙瓔tasisPoint 閹舵洘骞?
	if (AStasisPoint* StasisPoint = Cast<AStasisPoint>(ThrowHand->HeldActor))
	{
		HandleStasisPointThrow(ThrowHand, StasisPoint);
		return;
	}

	// 閸欘亝婀?GrabbeeObject 閹靛秴鍘戠拋鍛婂閹?
	AGrabbeeObject* ThrowObject = Cast<AGrabbeeObject>(ThrowHand->HeldActor);
	if (!ThrowObject)
	{
		return;
	}

	// 闁俺绻冪亸鍕殠鐠侊紕鐣婚幎鏇熷箯閻╊喗鐖ｉ悙鐧哥礄娴犲孩鎲氶崓蹇旀簚閺堟繂澧犻敍?
	FHitResult Hit;
	const bool bHit = PerformLineTrace(Hit, MaxThrowDistance, TCC_PROJECTILE);

	const FVector Start = FirstPersonCamera->GetComponentLocation();
	const FVector End = Start + FirstPersonCamera->GetForwardVector() * MaxThrowDistance;
	const FVector TargetPoint = bHit ? Hit.ImpactPoint : End;

	// 閸忓牓鍣撮弨鎾呯礄鐟欙綁娅?PhysicsHandle / 闂勫嫮娼冮敍澶涚礉閸愬秴褰傜亸?
	ThrowHand->ReleaseObject();

	// LaunchTowards 閸愬懘鍎存导姘闁喎瀹抽獮璺哄閸愭煡鍣?
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

	// 1) 鐠侊紕鐣婚崣鎴濈殸娑撳﹣绗呴弬鍥风礄PC閿涙艾鐔€娴滃海娴夐張鍝勫閸氭埊绱?
	const FVector CameraLocation = FirstPersonCamera->GetComponentLocation();
	const FVector CameraForward = FirstPersonCamera->GetForwardVector();

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);
	IgnoreActors.Add(StasisPoint);
	// 妫版繂顦婚惃鍕嫹閻ｃ儱顕挒鈽呯礄濮ｆ柨顩ч崣灞惧閹靛瀵旈悧鈺嬬礆閻?StasisPoint 閸愬懘鍎寸紒鎾虫値 HoldingHand 婢跺嫮鎮婇敓鏂ゆ嫹
	// 鏉╂瑩鍣锋禒宥勭箽閻ｆ瑨鐨熼悽銊ь伂閸欘垯绱堕崗銉ф畱 IgnoreActors 閹碘晛鐫嶉懗钘夊閵?

	// 2) 鐠侊紕鐣婚崚婵嬧偓鐔峰
	const FVector InitVelocity = CameraForward * StasisFireSpeedScalar;

	// 3) 闁插﹥鏂侀敍鍫Ｐ掗梽銈嗗閸欐牭绱?
	ThrowHand->ReleaseObject();

	// 4) 閸欐垵鐨犻敍姘辨暠鐎规俺闊╅悶鍐ㄥ敶闁劏鍤滅悰灞惧閻╊喗鐖ｉ敍灞惧娑撳秴鍩岄崚娆戞纯妞嬬偛鑻熺搾鍛閼奉亝鐦?
	StasisPoint->Fire(
		this,
		CameraLocation,
		CameraForward,
		InitVelocity,
		StasisDetectionRadius,
		StasisDetectionAngle,
		IgnoreActors
	);

	// 5) 鐟欙綁鏀ｉ幍瀣劥
	ThrowHand->SetGrabLock(false);
}

// ==================== 瀵挾顔勯幙宥勭稊 ====================

void ABasePCPlayer::StartAiming()
{
	if (bActionLocked)
	{
		return;
	}

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
	if (bActionLocked)
	{
		return;
	}

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
	if (bActionLocked)
	{
		return;
	}

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
	if (bActionLocked)
	{
		return;
	}

	// DEPRECATED: 娑撯偓閺冿箑绱戞慨瀣瀵挸姘ㄦ稉宥堝厴閸欐牗绉烽敍灞炬緱閹靛鍨ㄩ崚鍥ㄥ床濡€崇础闁垝绱伴惄瀛樺复閸欐垵鐨?
	// 濮濄倕鍤遍弫棰佺箽閻ｆ瑧鏁ゆ禍搴″悑鐎圭櫢绱濇担鍡楀敶闁劎娲块幒銉ㄧ殶閻?ReleaseBowString
	if (bIsDrawingBow)
	{
		CancelDrawBow();
	}
}

void ABasePCPlayer::ReleaseBowString()
{
	if (bActionLocked)
	{
		return;
	}

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

// ==================== 閸愬懘鍎撮崙鑺ユ殶 ====================

void ABasePCPlayer::UpdateTargetDetection()
{
	FHitResult Hit;
	AActor* NewTarget = nullptr;
	FName NewBoneName = NAME_None;
	FVector NewImpactPoint = FVector::ZeroVector;
	UPrimitiveComponent* NewHitComponent = nullptr;

	bTraceHit = PerformLineTrace(Hit, MaxGrabDistance, GrabTraceChannel);

	if (false && bTraceHit)	
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Grab Trace Hit: %s"), *Hit.GetActor()->GetName()));
	
	bool bSightHitsIgniteTarget = false;
	bCanIgniteBySight = false;
	IgniteBySightImpactPoint = FVector::ZeroVector;
	if (bTraceHit)
	{
		NewHitComponent = Hit.GetComponent();
		if (UPrimitiveComponent* HitComp = NewHitComponent)
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
		TargetedHitComponent = NewHitComponent;
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
		TargetedHitComponent = NewHitComponent;

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
		TargetedHitComponent = NewHitComponent;
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
	// 瑜版挷鎹㈡稉鈧崣顏呭閹舵挸褰囬悧鈺€缍嬮弮璁圭礉缁斿宓嗗〒鍛敄閻嫬鍣惄顔界垼
	if (TargetedObject && IsValid(TargetedObject))
	{
		AActor* OldTarget = TargetedObject;
		TargetedObject = nullptr;
		TargetedBoneName = NAME_None;
		TargetedImpactPoint = FVector::ZeroVector;
		TargetedHitComponent = nullptr;

		// 閸欐牗绉烽柅澶夎厬閻樿埖鈧緤绱欓柅姘崇箖閹恒儱褰涢敍?
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


