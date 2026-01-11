// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/BaseVRPlayer.h"
#include "Grabber/VRGrabHand.h"
#include "Camera/CameraComponent.h"
#include "MotionControllerComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetInteractionComponent.h"

ABaseVRPlayer::ABaseVRPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建 VR 原点
	VROrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	VROrigin->SetupAttachment(RootComponent);

	// 创建 VR 摄像机
	VRCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	VRCamera->SetupAttachment(VROrigin);

	// 创建背包碰撞区域（在蓝图设置变换）
	BackpackCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BackpackCollision"));
	BackpackCollision->SetupAttachment(VRCamera);
	BackpackCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BackpackCollision->SetCollisionResponseToAllChannels(ECR_Overlap);
	BackpackCollision->SetGenerateOverlapEvents(true);
	BackpackCollision->ComponentTags.Add(FName("player_backpack"));

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
	LeftHandCollision = CreateDefaultSubobject<USphereComponent>(TEXT("LeftHandCollision"));
	LeftHandCollision->SetupAttachment(VRLeftHand);
	LeftHandCollision->SetSphereRadius(5.0f);
	LeftHandCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	LeftHandCollision->SetCollisionResponseToAllChannels(ECR_Overlap);
	LeftHandCollision->SetGenerateOverlapEvents(true);
	LeftHandCollision->ComponentTags.Add(FName("player_hand"));
	VRLeftHand->HandCollision = LeftHandCollision;

	// 创建右手抓取组件
	VRRightHand = CreateDefaultSubobject<UVRGrabHand>(TEXT("RightHand"));
	VRRightHand->SetupAttachment(MotionControllerRight);
	VRRightHand->bIsRightHand = true;
	RightHand = VRRightHand;  // 赋值给 BasePlayer 的基类指针


	// 创建右手碰撞体
	RightHandCollision = CreateDefaultSubobject<USphereComponent>(TEXT("RightHandCollision"));
	RightHandCollision->SetupAttachment(VRRightHand);
	RightHandCollision->SetSphereRadius(5.0f);
	RightHandCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RightHandCollision->SetCollisionResponseToAllChannels(ECR_Overlap);
	RightHandCollision->SetGenerateOverlapEvents(true);
	RightHandCollision->ComponentTags.Add(FName("player_hand"));
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
