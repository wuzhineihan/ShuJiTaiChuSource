// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/BaseVRPlayer.h"
#include "Grabber/VRGrabHand.h"
#include "Camera/CameraComponent.h"
#include "MotionControllerComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "Skill/PlayerSkillComponent.h"
#include "Game/CollisionConfig.h"
#include "Components/CapsuleComponent.h"

ABaseVRPlayer::ABaseVRPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建 VR 原点
	VROrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	VROrigin->SetupAttachment(RootComponent);

	// 创建 VR 摄像机
	VRCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	VRCamera->SetupAttachment(VROrigin);
	PlayerCamera = VRCamera;

	// Camera collision (probe)
	CameraCollision = CreateDefaultSubobject<USphereComponent>(TEXT("CameraCollision"));
	CameraCollision->SetupAttachment(VRCamera);
	CameraCollision->InitSphereRadius(CameraCollisionRadius);
	CameraCollision->SetCollisionProfileName(CP_PLAYER_CAMERA_COLLISION);
	CameraCollision->SetGenerateOverlapEvents(true);
	CameraCollision->SetCanEverAffectNavigation(false);

	// 创建背包碰撞区域（在蓝图设置变换）
	BackpackCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BackpackCollision"));
	BackpackCollision->SetupAttachment(VRCamera);
	BackpackCollision->SetCollisionProfileName(CP_PLAYER_BACKPACK);
	BackpackCollision->SetGenerateOverlapEvents(true);
	BackpackCollision->OnComponentBeginOverlap.AddDynamic(this, &ABaseVRPlayer::OnBackpackBeginOverlap);
	BackpackCollision->OnComponentEndOverlap.AddDynamic(this, &ABaseVRPlayer::OnBackpackEndOverlap);

	// 创建左手 MotionController
	MotionControllerLeft = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionControllerLeft"));
	MotionControllerLeft->SetupAttachment(VROrigin);
	MotionControllerLeft->SetTrackingMotionSource(FName("Left"));

	// 创建右手 MotionController
	MotionControllerRight = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionControllerRight"));
	MotionControllerRight->SetupAttachment(VROrigin);
	MotionControllerRight->SetTrackingMotionSource(FName("Right"));

	// 创建左手 Aim MotionController
	MotionControllerLeftAim = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionControllerLeftAim"));
	MotionControllerLeftAim->SetupAttachment(VROrigin);
	MotionControllerLeftAim->SetTrackingMotionSource(FName("LeftAim"));

	// 创建右手 Aim MotionController
	MotionControllerRightAim = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionControllerRightAim"));
	MotionControllerRightAim->SetupAttachment(VROrigin);
	MotionControllerRightAim->SetTrackingMotionSource(FName("RightAim"));

	// 创建左手抓取组件
	VRLeftHand = CreateDefaultSubobject<UVRGrabHand>(TEXT("LeftHand"));
	VRLeftHand->SetupAttachment(MotionControllerLeft);
	VRLeftHand->bIsRightHand = false;
	LeftHand = VRLeftHand;  // 赋值给 BasePlayer 的基类指针

	// 创建左手碰撞体
	USphereComponent* LeftHandCollision = CreateDefaultSubobject<USphereComponent>(TEXT("LeftHandCollision"));
	LeftHandCollision->SetupAttachment(VRLeftHand);
	LeftHandCollision->SetSphereRadius(5.0f);
	LeftHandCollision->SetCollisionProfileName(CP_PLAYER_HAND);
	LeftHandCollision->SetGenerateOverlapEvents(true);
	VRLeftHand->HandCollision = LeftHandCollision;

	// 创建右手抓取组件
	VRRightHand = CreateDefaultSubobject<UVRGrabHand>(TEXT("RightHand"));
	VRRightHand->SetupAttachment(MotionControllerRight);
	VRRightHand->bIsRightHand = true;
	RightHand = VRRightHand;  // 赋值给 BasePlayer 的基类指针

	// 创建右手碰撞体
	USphereComponent* RightHandCollision = CreateDefaultSubobject<USphereComponent>(TEXT("RightHandCollision"));
	RightHandCollision->SetupAttachment(VRRightHand);
	RightHandCollision->SetSphereRadius(5.0f);
	RightHandCollision->SetCollisionProfileName(CP_PLAYER_HAND);
	RightHandCollision->SetGenerateOverlapEvents(true);
	VRRightHand->HandCollision = RightHandCollision;

	// 设置双手引用
	VRLeftHand->OtherHand = VRRightHand;
	VRRightHand->OtherHand = VRLeftHand;

	// 创建左手 Widget 交互
	WidgetInteractionLeft = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("WidgetInteractionLeft"));
	WidgetInteractionLeft->SetupAttachment(MotionControllerLeftAim);
	WidgetInteractionLeft->bShowDebug = false;  // 禁用调试可视化

	// 创建右手 Widget 交互
	WidgetInteractionRight = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("WidgetInteractionRight"));
	WidgetInteractionRight->SetupAttachment(MotionControllerRightAim);
	WidgetInteractionRight->bShowDebug = false;  // 禁用调试可视化
}

void ABaseVRPlayer::BeginPlay()
{
	Super::BeginPlay();
}

void ABaseVRPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 输入绑定在蓝图中配置（Enhanced Input）
}

void ABaseVRPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateCapsuleToCamera();
}

void ABaseVRPlayer::UpdateCapsuleToCamera()
{
	if (!VRCamera || !VROrigin)
	{
		return;
	}

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (!Capsule)
	{
		return;
	}

	// ==================== Z：根据相机相对高度动态设置胶囊体半高，并修正 VROrigin 相对Z ====================
	// 说明：这里用相机相对 VROrigin 的高度（HMD 高度），避免把世界坐标引入计算。
	const float HMDHeight = VRCamera->GetRelativeLocation().Z;
	const float ClampedHeight = FMath::Clamp(HMDHeight, 20.f, 1000.f);
	const float TargetHalfHeight = ClampedHeight * 0.5f;

	Capsule->SetCapsuleHalfHeight(TargetHalfHeight, false);

	// ��囊体半高变化会以中心缩放，为保持“胶囊底部=地面基准”，让 VROrigin 的相对Z = -HalfHeight
	FVector OriginRelLoc = VROrigin->GetRelativeLocation();
	OriginRelLoc.Z = -TargetHalfHeight;
	VROrigin->SetRelativeLocation(OriginRelLoc);

	// ==================== XY：将胶囊体水平对齐到相机，同时反向偏移 VROrigin ====================
	const FVector CameraLoc = VRCamera->GetComponentLocation();
	const FVector CapsuleLoc = Capsule->GetComponentLocation();
	FVector DeltaLoc = CameraLoc - CapsuleLoc;
	DeltaLoc.Z = 0.f;

	if (DeltaLoc.IsNearlyZero())
	{
		return;
	}

	Capsule->AddWorldOffset(DeltaLoc, false, nullptr, ETeleportType::None);
	VROrigin->AddWorldOffset(-DeltaLoc, false, nullptr, ETeleportType::None);
}

// ==================== 输入处理 ====================

void ABaseVRPlayer::HandleLeftGrip(bool bPressed)
{
	HandleGrip(VRLeftHand, bPressed, true);
}

void ABaseVRPlayer::HandleRightGrip(bool bPressed)
{
	HandleGrip(VRRightHand, bPressed, false);
}

// ==================== 内部函数 ====================

void ABaseVRPlayer::HandleGrip(UVRGrabHand* Hand, bool bPressed, bool bIsLeft)
{
	if (!Hand)
	{
		return;
	}

	// 弓箭模式下，左手持弓，无视Grip操作
	if (bIsLeft && bIsBowArmed)
	{
		return;
	}

	// 绘制互斥：VR 只禁用“正在绘制的那只手”的抓取
	if (PlayerSkillComponent && PlayerSkillComponent->IsDrawing())
	{
		const bool bDrawingRight = PlayerSkillComponent->IsRightHandDrawing();
		const bool bThisHandIsRight = Hand->bIsRightHand;
		if (bThisHandIsRight == bDrawingRight)
		{
			return;
		}
	}

	if (bPressed)
	{
		Hand->TryGrab();
	}
	else
	{
		Hand->TryRelease();
	}
}

// ==================== 重写基类 ====================

void ABaseVRPlayer::PlaySimpleForceFeedback(EControllerHand Hand)
{
	Super::PlaySimpleForceFeedback(Hand);
	PlayerController->PlayHapticEffect(SimpleHapticEffect, Hand);
}

// =================== Tools ====================
void ABaseVRPlayer::SetVRHandRotationOffset(FRotator RotationOffset)
{
	VRLeftHand->SetRelativeRotation(RotationOffset);
	RotationOffset.Yaw *= -1.f;
	RotationOffset.Roll *= -1.f;
	VRRightHand->SetRelativeRotation(RotationOffset);
}

void ABaseVRPlayer::OnBackpackBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherComp || OtherComp->GetCollisionObjectType() != OCC_PLAYER_HAND)
	{
		return;
	}

	if (UVRGrabHand* Hand = GetHandFromCollision(OtherComp))
	{
		Hand->bIsInBackpackArea = true;
		PlaySimpleForceFeedback(Hand->bIsRightHand ? EControllerHand::Right : EControllerHand::Left);
	}
}

void ABaseVRPlayer::OnBackpackEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherComp || OtherComp->GetCollisionObjectType() != OCC_PLAYER_HAND)
	{
		return;
	}

	if (UVRGrabHand* Hand = GetHandFromCollision(OtherComp))
	{
		Hand->bIsInBackpackArea = false;
	}
}

UVRGrabHand* ABaseVRPlayer::GetHandFromCollision(UPrimitiveComponent* Comp) const
{
	if (!Comp)
	{
		return nullptr;
	}

	return Cast<UVRGrabHand>(Comp->GetAttachParent());
}
