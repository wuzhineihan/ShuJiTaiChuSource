// Fill out your copyright notice in the Description page of Project Settings.

#include "Grabbee/Bow.h"
#include "Grabbee/Arrow.h"
#include "Grabber/PCGrabHand.h"
#include "Grabber/PlayerGrabHand.h"
#include "Game/Characters/BasePlayer.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "Audio/AudioSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Game/CollisionConfig.h"
#include "Game/MyGameplayTags.h"

ABow::ABow()
{
	PrimaryActorTick.bCanEverTick = true;

	// 璁剧疆姝﹀櫒绫诲瀷
	WeaponType = EWeaponType::Bow;
	
	// 寮撲娇鐢?WeaponSnap 绫诲瀷锛圓ttach 鍒版墜涓婏級
	GrabType = EGrabType::WeaponSnap;

	// 鍒涘缓寮撳鸡缃戞牸浣?
	StringMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StringMesh"));
	StringMesh->SetupAttachment(MeshComponent);
	StringMesh->SetCollisionProfileName(CP_NO_COLLISION);

	// 鍒涘缓寮撳鸡纰版挒鍖哄煙
	StringCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("StringCollision"));
	StringCollision->SetupAttachment(StringMesh);
	StringCollision->SetBoxExtent(FVector(5.0f, 5.0f, 5.0f));
	StringCollision->SetCollisionProfileName(CP_BOW_STRING_COLLISION);
	StringCollision->OnComponentBeginOverlap.AddDynamic(this, &ABow::OnStringCollisionBeginOverlap);
	StringCollision->OnComponentEndOverlap.AddDynamic(this, &ABow::OnStringCollisionEndOverlap);

	// 鍒涘缓寮撳墠绔綅缃爣璁?
	BowFrontPosition = CreateDefaultSubobject<USceneComponent>(TEXT("BowFrontPosition"));
	BowFrontPosition->SetupAttachment(MeshComponent);
	BowFrontPosition->SetRelativeLocation(FVector(50.0f, 0.0f, 0.0f)); // 寮撶殑鍓嶆柟

	// 鍒涘缓寮撳鸡榛樿浣嶇疆
	StringRestPosition = CreateDefaultSubobject<USceneComponent>(TEXT("StringRestPosition"));
	StringRestPosition->SetupAttachment(MeshComponent);

	// 鍒涘缓杞ㄨ抗棰勮
	ArrowTracePreview = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrajectoryPreview"));
	ArrowTracePreview->SetupAttachment(StringMesh);
	ArrowTracePreview->SetAutoActivate(false);
	ArrowTracePreview->SetVisibility(false);

	// 璁剧疆榛樿鏍囩
	Tags.Add(FName("Bow"));
}

void ABow::BeginPlay()
{
	Super::BeginPlay();

	// 鍒涘缓寮撳鸡鍔ㄦ€佹潗璐?
	if (StringMesh && StringMesh->GetMaterial(0))
	{
		StringMID = StringMesh->CreateAndSetMaterialInstanceDynamic(0);
	}

	// 鍒濆鍖栧紦寮︿綅缃?
	CurrentGrabSpot = StringRestPosition ? StringRestPosition->GetComponentLocation() : StringMesh->GetComponentLocation();
}

void ABow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 鏇存柊寮撳鸡浣嶇疆
	UpdateStringPosition(DeltaTime);

	// 鏇存柊绠殑浣嶇疆
	UpdateArrowPosition();
}

// ==================== 鏍稿績鎺ュ彛 ====================

void ABow::ReleaseString()
{
    // 濡傛灉鏈夌锛屽彂灏?
    if (NockedArrow)
    {
        FireArrow();
    }

    bStringHeld = false;
    StringHoldingHand = nullptr;

    // 闅愯棌杞ㄨ抗棰勮
    if (ArrowTracePreview)
    {
        ArrowTracePreview->SetVisibility(false);
    }
}

bool ABow::NockArrow(AArrow* Arrow)
{
	if (!Arrow)
	{
		return false;
	}

	// 濡傛灉宸叉湁绠紝鍏堝彇娑?
	if (NockedArrow)
	{
		UnnockArrow();
	}

	NockedArrow = Arrow;
	NockedArrow->EnterNockedState(this);

	// OwningCharacter 宸插湪绠鎶撳彇鏃惰缃紝鏃犻渶閲嶅璧嬪€?

	return true;
}

void ABow::UnnockArrow()
{
	if (NockedArrow)
	{
		NockedArrow->EnterIdleState();
		NockedArrow = nullptr;
	}
}


void ABow::FireArrow()
{
    if (!NockedArrow)
    {
        return;
    }

    // 璁＄畻鍙戝皠閫熷害
    float FiringSpeed = CalculateFiringSpeed();

    // 鍙戝皠绠?
    NockedArrow->EnterFlyingState(FiringSpeed);
	
	CachedAudioSubsystem->PlayNormalSound2D(MyProjectTags::TAG_NormalSound_ArrowShoot);
	
    // 娓呴櫎寮曠敤
    NockedArrow = nullptr;

    // 闅愯棌杞ㄨ抗棰勮
    if (ArrowTracePreview)
    {
        ArrowTracePreview->SetVisibility(false);
    }
}


float ABow::CalculateFiringSpeed() const
{
	float Speed = CurrentPullLength * FiringSpeedMultiplier;
	return FMath::Clamp(Speed, MinFiringSpeed, MaxFiringSpeed);
}

void ABow::UpdateArrowTracePreview()
{
	if (!NockedArrow || !ArrowTracePreview)
	{
		return;
	}

	// 璁＄畻鍙戝皠鏂瑰悜
	FVector LaunchDirection = BowFrontPosition->GetComponentLocation() - CurrentGrabSpot;
	LaunchDirection.Normalize();

	// 璁＄畻鍙戝皠閫熷害
	float Speed = CalculateFiringSpeed();
	FVector LaunchVelocity = LaunchDirection * Speed;

	// 棰勬祴杞ㄨ抗
	FPredictProjectilePathParams PathParams;
	PathParams.StartLocation = CurrentGrabSpot;
	PathParams.LaunchVelocity = LaunchVelocity;
	PathParams.ProjectileRadius = 2.0f;
	PathParams.MaxSimTime = 3.0f;
	PathParams.bTraceWithCollision = true;
	PathParams.bTraceComplex = false;
	PathParams.TraceChannel = TCC_PROJECTILE;
	PathParams.ActorsToIgnore.Add(this);
	PathParams.ActorsToIgnore.Add(BowOwner);
	if (NockedArrow)
	{
		PathParams.ActorsToIgnore.Add(NockedArrow);
	}

	FPredictProjectilePathResult PathResult;
	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);

	// 灏嗚建杩圭偣浼犻€掔粰 Niagara
	TArray<FVector> PathPoints;
	for (const FPredictProjectilePathPointData& PointData : PathResult.PathData)
	{
		PathPoints.Add(PointData.Location);
	}

	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(ArrowTracePreview, FName("User.PointArray"), PathPoints);
}

// ==================== 閲嶅啓 ====================

EGrabType ABow::GetGrabType_Implementation() const
{
	if (!bBodyHeld)
	{
		// 寮撹韩鏈鎶?鈫?姝ｅ父姝﹀櫒鎶撳彇
		return EGrabType::WeaponSnap;
	}
	else
	{
		// 寮撹韩宸茶鎶?鈫?鑷畾涔夊鐞嗗紦寮?
		return EGrabType::Custom;
	}
}

UPrimitiveComponent* ABow::GetGrabPrimitive_Implementation() const
{
	if (!bBodyHeld)
	{
		return MeshComponent;
	}
	return nullptr;  // 寮撳鸡涓嶉渶瑕?Primitive
}

bool ABow::SupportsDualHandGrab_Implementation() const
{
	return true;  // 寮撴敮鎸佸弻鎵嬫姄鍙栵紙寮撹韩 + 寮撳鸡锛?
}

bool ABow::CanBeGrabbedBy_Implementation(const UPlayerGrabHand* Hand) const
{
	if (!Hand)
	{
		return false;
	}

	if (bBodyHeld && Cast<UPCGrabHand>(Hand))
	{
		return false;
	}

	// 濡傛灉寮撹韩鏈鎶撳彇锛屾甯告鏌?
	if (!bBodyHeld)
	{
		return bCanGrab && !bIsHeld;
	}

	// 寮撹韩宸茶鎶撳彇锛屾鏌ユ槸鍚﹀彲浠ユ姄寮撳鸡
	// 寮撳鸡鏈鎶撳彇 涓?涓嶆槸鍚屼竴鍙墜 涓?鎵嬪湪寮撳鸡纰版挒鍖哄煙鍐?
	if (!bStringHeld && Hand != BodyHoldingHand && InStringCollisionHand)
	{
		return true;
	}

	return false;
}

void ABow::OnGrabbed_Implementation(UPlayerGrabHand* Hand)
{
	if (!Hand)
	{
		return;
	}

	// 鍒ゆ柇鏄姄寮撹韩杩樻槸鎶撳紦寮?
	if (!bBodyHeld)
	{
		// 绗竴娆℃姄鍙栵細鎶撳紦韬?
		Super::OnGrabbed_Implementation(Hand);

		bBodyHeld = true;
		BodyHoldingHand = Hand;

		// 灏濊瘯鑾峰彇寮撶殑鎸佹湁鑰?
		AActor* HandOwner = Hand->GetOwner();
		if (ABasePlayer* Player = Cast<ABasePlayer>(HandOwner))
		{
			BowOwner = Player;
		}
	}
	else if (!bStringHeld && Hand != BodyHoldingHand)
	{
		bStringHeld = true;
		StringHoldingHand = Hand;

		// 璁板綍鎶撳彇鏃剁殑鍋忕Щ锛堝紦寮︿綅缃浉瀵逛簬鎵嬮儴浣嶇疆锛?
		FVector StringPos = StringRestPosition ? StringRestPosition->GetComponentLocation() : StringMesh->GetComponentLocation();
		InitialStringGrabOffset = StringPos - Hand->GetComponentLocation();

		// 鏄剧ず杞ㄨ抗棰勮
		if (NockedArrow && ArrowTracePreview)
		{
			ArrowTracePreview->SetVisibility(true);
		}
	}
}

void ABow::OnReleased_Implementation(UPlayerGrabHand* Hand)
{
	// 妫€鏌ユ槸閲婃斁寮撹韩杩樻槸寮撳鸡
	if (Hand == BodyHoldingHand)
	{
		// 閲婃斁寮撹韩
		Super::OnReleased_Implementation(Hand);

		bBodyHeld = false;
		BodyHoldingHand = nullptr;

		// 鍚屾椂閲婃斁寮撳鸡
		if (bStringHeld)
		{
			ReleaseString();
		}
	}
	else if (Hand == StringHoldingHand)
	{
		// 閲婃斁寮撳鸡锛堝彂灏勭锛?
		ReleaseString();
	}
}

// ==================== 鍐呴儴鍑芥暟 ====================

UPlayerGrabHand* ABow::GetHandFromCollision(UPrimitiveComponent* Comp) const
{
	if (!Comp)
	{
		return nullptr;
	}
	
	// HandCollision 鏄?PlayerGrabHand 鐨勫瓙缁勪欢
	if (UPlayerGrabHand* Hand = Cast<UPlayerGrabHand>(Comp->GetAttachParent()))
	{
		return Hand;
	}
	
	return nullptr;
}

void ABow::OnStringCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 妫€鏌ユ槸鍚︽槸鐜╁鐨勬墜
	if (OtherComp && OtherComp->GetCollisionObjectType() == OCC_PLAYER_HAND)
	{
		// 鑾峰彇鎵嬬粍浠?
		UPlayerGrabHand* Hand = GetHandFromCollision(OtherComp);
		TryHandleStringHandEnter(Hand);
	}
}

void ABow::TryHandleStringHandEnter(UPlayerGrabHand* Hand)
{
	if (!Hand)
	{
		return;
	}

	if (Cast<UPCGrabHand>(Hand))
	{
		return;
	}

	InStringCollisionHand = Hand;

	// If same hand as bow body / body not held / string already held, skip.
	if (Hand == BodyHoldingHand || !bBodyHeld || bStringHeld)
	{
		return;
	}

	if (BowOwner)
	{
		BowOwner->PlaySimpleForceFeedback(Hand->bIsRightHand ? EControllerHand::Right : EControllerHand::Left);
	}

	if (AArrow* Arrow = Cast<AArrow>(Hand->HeldActor))
	{
		Hand->ReleaseObject();
		if (!NockArrow(Arrow))
		{
			Hand->GrabObject(Arrow);
			return;
		}
	}
	else if (!NockedArrow)
	{
		return;
	}

	Hand->GrabObject(this);
}

void ABow::OnStringCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// 妫€鏌ユ槸鍚︽槸鐜╁鐨勬墜绂诲紑
	if (OtherComp && OtherComp->GetCollisionObjectType() == OCC_PLAYER_HAND)
	{
		InStringCollisionHand = nullptr;
	}
}

void ABow::UpdateStringPosition(float DeltaTime)
{
    FVector TargetPos;

    if (bStringHeld)
    {
        // 寮撳鸡琚媺鍔?
        if (StringHoldingHand)
        {
            // 璺熼殢鎵嬬殑浣嶇疆
            TargetPos = StringHoldingHand->GetComponentLocation() + InitialStringGrabOffset;
        }
        else
        {
            TargetPos = StringRestPosition->GetComponentLocation();
        }

        // 闄愬埗鎷夊鸡璺濈
        FVector RestPos = StringRestPosition ? StringRestPosition->GetComponentLocation() : StringMesh->GetComponentLocation();
        FVector PullVector = TargetPos - RestPos;
        float PullDist = PullVector.Size();

        if (PullDist > MaxPullDistance)
        {
            TargetPos = RestPos + PullVector.GetSafeNormal() * MaxPullDistance;
            PullDist = MaxPullDistance;
        	
            if (!bPlayedTightSound)
            {
                if (CachedAudioSubsystem)
                {
                    CachedAudioSubsystem->PlayNormalSound2D(MyProjectTags::TAG_NormalSound_BowStringTight);
                }
                bPlayedTightSound = true;
            }
        }
        else
        {
            bPlayedTightSound = false;
        }

        CurrentGrabSpot = TargetPos;
        CurrentPullLength = PullDist;

        // 鏇存柊寮撳鸡鏉愯川
        if (StringMID)
        {
            StringMID->SetVectorParameterValue(FName("GrabSpot"), FLinearColor(CurrentGrabSpot));
        }

        // 鏇存柊杞ㄨ抗棰勮
        if (NockedArrow)
        {
            UpdateArrowTracePreview();
        }
    }
    else
    {
        // 寮撳鸡鍥炲脊
        TargetPos = StringRestPosition ? StringRestPosition->GetComponentLocation() : StringMesh->GetComponentLocation();
        CurrentGrabSpot = SpringSolve(CurrentGrabSpot, TargetPos, StringSpringStrength, StringSpringDamping, DeltaTime);
        CurrentPullLength = (CurrentGrabSpot - TargetPos).Size();

        // 鏇存柊寮撳鸡鏉愯川
        if (StringMID)
        {
            StringMID->SetVectorParameterValue(FName("GrabSpot"), FLinearColor(CurrentGrabSpot));
        }
    }
}

void ABow::UpdateArrowPosition()
{
	if (!NockedArrow)
	{
		return;
	}

	// 璁＄畻绠殑浣嶇疆鍜屾湞鍚?
	FVector ArrowLocation = CurrentGrabSpot;
	FVector Direction = BowFrontPosition->GetComponentLocation() - CurrentGrabSpot;
	FRotator ArrowRotation = UKismetMathLibrary::MakeRotFromX(Direction);

	NockedArrow->SetActorLocationAndRotation(ArrowLocation, ArrowRotation);
}

FVector ABow::SpringSolve(const FVector& Current, const FVector& Target, float Strength, float Damping, float DeltaTime)
{
	// 寮圭哀鍏紡锛欶 = -k * x - d * v
	FVector Displacement = Current - Target;
	FVector SpringForce = -Strength * Displacement;
	FVector DampingForce = -Damping * StringVelocity;
	FVector Acceleration = SpringForce + DampingForce;

	StringVelocity += Acceleration * DeltaTime;
	FVector NewPosition = Current + StringVelocity * DeltaTime;

	// 濡傛灉瓒冲鎺ヨ繎鐩爣锛屽仠姝?
	if (Displacement.Size() < 0.1f && StringVelocity.Size() < 0.1f)
	{
		StringVelocity = FVector::ZeroVector;
		return Target;
	}

	return NewPosition;
}
