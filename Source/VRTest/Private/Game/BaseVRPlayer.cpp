// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/BaseVRPlayer.h"
#include "Game/VRGrabHand.h"
#include "Game/InventoryComponent.h"
#include "Grabbee/GrabbeeWeapon.h"
#include "Camera/CameraComponent.h"
#include "MotionControllerComponent.h"
#include "Components/BoxComponent.h"
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

	// 创建背包碰撞区域（附加到摄像机后方）
	BackpackCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BackpackCollision"));
	BackpackCollision->SetupAttachment(VRCamera);
	BackpackCollision->SetRelativeLocation(FVector(-30.0f, 0.0f, -20.0f));
	BackpackCollision->SetBoxExtent(FVector(15.0f, 20.0f, 25.0f));
	BackpackCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BackpackCollision->SetCollisionResponseToAllChannels(ECR_Overlap);

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
	LeftHand = CreateDefaultSubobject<UVRGrabHand>(TEXT("LeftHand"));
	LeftHand->SetupAttachment(MotionControllerLeft);
	LeftHand->bIsRightHand = false;

	// 创建右手抓取组件
	RightHand = CreateDefaultSubobject<UVRGrabHand>(TEXT("RightHand"));
	RightHand->SetupAttachment(MotionControllerRight);
	RightHand->bIsRightHand = true;

	// 设置双手引用
	LeftHand->OtherHand = RightHand;
	RightHand->OtherHand = LeftHand;

	// 创建左手 Widget 交互
	WidgetInteractionLeft = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("WidgetInteractionLeft"));
	WidgetInteractionLeft->SetupAttachment(MotionControllerLeftAim);

	// 创建右手 Widget 交互
	WidgetInteractionRight = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("WidgetInteractionRight"));
	WidgetInteractionRight->SetupAttachment(MotionControllerRightAim);
}

void ABaseVRPlayer::BeginPlay()
{
	Super::BeginPlay();

	// 设置手部的背包碰撞引用
	SetupHandBackpackReferences();
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
	// 弓箭模式下，左手持弓，无视Grip操作
	if (bIsBowArmed)
	{
		return;
	}

	if (bPressed)
	{
		// 检查是否在背包区域
		if (LeftHand->IsInBackpackArea())
		{
			LeftHand->TryGrab(true); // 从背包取物
		}
		else
		{
			LeftHand->TryGrab(false); // 正常抓取
		}
	}
	else
	{
		LeftHand->TryRelease();
	}
}

void ABaseVRPlayer::HandleRightGrip(bool bPressed)
{
	if (bPressed)
	{
		// 检查是否在背包区域
		if (RightHand->IsInBackpackArea())
		{
			RightHand->TryGrab(true); // 从背包取物
		}
		else
		{
			RightHand->TryGrab(false); // 正常抓取
		}
	}
	else
	{
		RightHand->TryRelease();
	}
}

// ==================== 内部函数 ====================

void ABaseVRPlayer::SetupHandBackpackReferences()
{
	if (LeftHand)
	{
		LeftHand->BackpackCollision = BackpackCollision;
	}
	if (RightHand)
	{
		RightHand->BackpackCollision = BackpackCollision;
	}
}

// ==================== 重写基类 ====================

void ABaseVRPlayer::SetBowArmed(bool bArmed)
{
	// 退出弓箭模式时先释放左手
	if (bIsBowArmed && !bArmed)
	{
		if (LeftHand && LeftHand->bIsHolding && LeftHand->HeldObject == CurrentBow)
		{
			// 释放附加但不触发 OnReleased（弓会被销毁）
			LeftHand->ReleaseAttachOnly();
		}
	}

	// 调用基类处理弓的生成/销毁
	Super::SetBowArmed(bArmed);

	// 进入弓箭模式时使用 GrabHand 的抓取逻辑
	if (bArmed && CurrentBow && LeftHand)
	{
		LeftHand->GrabObject(CurrentBow);
	}
}
