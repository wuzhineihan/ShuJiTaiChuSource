// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/BasePCPlayer.h"
#include "Game/PCGrabHand.h"
#include "Game/InventoryComponent.h"
#include "Grabbee/GrabbeeWeapon.h"
#include "Camera/CameraComponent.h"

ABasePCPlayer::ABasePCPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建第一人称摄像机
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(RootComponent);
	FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
	FirstPersonCamera->bUsePawnControlRotation = true;

	// 创建左手
	LeftHand = CreateDefaultSubobject<UPCGrabHand>(TEXT("LeftHand"));
	LeftHand->SetupAttachment(FirstPersonCamera);
	LeftHand->bIsRightHand = false;

	// 创建右手
	RightHand = CreateDefaultSubobject<UPCGrabHand>(TEXT("RightHand"));
	RightHand->SetupAttachment(FirstPersonCamera);
	RightHand->bIsRightHand = true;

	// 设置双手引用
	LeftHand->OtherHand = RightHand;
	RightHand->OtherHand = LeftHand;
}

void ABasePCPlayer::BeginPlay()
{
	Super::BeginPlay();
}

void ABasePCPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	// 输入绑定在蓝图中配置（Enhanced Input）
}

void ABasePCPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// ==================== 重写基类 ====================

void ABasePCPlayer::SetBowArmed(bool bArmed)
{
	// 退出弓箭模式时的清理
	if (bIsBowArmed && !bArmed)
	{
		// 停止瞄准和拉弓
		if (bIsAiming)
		{
			StopAiming();
		}
		if (bIsDrawingBow)
		{
			// 取消拉弓但不发射
			bIsDrawingBow = false;
			RightHand->InterpToDefaultTransform();
		}
		
		// 释放左手持有的弓（不触发 OnReleased，因为弓会被销毁）
		if (LeftHand->bIsHolding && LeftHand->HeldObject == CurrentBow)
		{
			LeftHand->ReleaseAttachOnly();
		}
	}

	// 调用基类处理弓的生成/销毁
	Super::SetBowArmed(bArmed);

	// 进入弓箭模式时使用 GrabHand 的抓取逻辑
	if (bArmed && CurrentBow)
	{
		LeftHand->GrabObject(CurrentBow);
	}
}

// ==================== 输入处理 ====================

void ABasePCPlayer::HandleLeftTrigger(bool bPressed)
{
	if (!bIsBowArmed)
	{
		// 徒手模式
		if (bPressed)
		{
			LeftHand->TryGrabOrRelease();
		}
	}
	else
	{
		// 弓箭模式
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

void ABasePCPlayer::HandleRightTrigger(bool bPressed)
{
	if (!bIsBowArmed)
	{
		// 徒手模式
		if (bPressed)
		{
			RightHand->TryGrabOrRelease();
		}
	}
	else
	{
		// 弓箭模式
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

void ABasePCPlayer::HandleModeSwitch(bool bToBowMode)
{
	SetBowArmed(bToBowMode);
}

// ==================== 弓箭操作 ====================

void ABasePCPlayer::StartAiming()
{
	if (!bIsBowArmed || !bHasBow)
	{
		return;
	}

	bIsAiming = true;
	
	// 将左手平滑过渡到瞄准位置
	LeftHand->InterpToTransform(AimingLeftHandTransform);
}

void ABasePCPlayer::StopAiming()
{
	bIsAiming = false;
	
	// 左手回到默认位置
	LeftHand->InterpToDefaultTransform();
}

void ABasePCPlayer::StartDrawBow()
{
	if (!bIsAiming)
	{
		return;
	}

	// 检查是否有箭
	if (!InventoryComponent || !InventoryComponent->HasArrow())
	{
		PlayNoArrowSound();
		return;
	}

	bIsDrawingBow = true;

	// 1. 计算目标拉弓距离
	float TargetDistance = CalculateDrawDistance();

	// 2. 计算右手目标位置（相对于摄像机）
	// 拉弓时右手在弓弦位置向后拉
	FTransform DrawTransform;
	DrawTransform.SetLocation(FVector(-TargetDistance, 0.0f, 0.0f)); // 向后拉
	
	// 3. 程序化拉弓
	RightHand->InterpToTransform(DrawTransform);

	// 4. 消耗一支箭（实际发射时生成）
	// 箭的消耗在 ReleaseBowString 时处理
}

void ABasePCPlayer::ReleaseBowString()
{
	if (!bIsDrawingBow)
	{
		return;
	}

	bIsDrawingBow = false;

	// TODO: 发射箭矢
	// 这里需要调用弓的 FireArrow 函数
	// CurrentBow->ReleaseString();

	// 消耗箭
	if (InventoryComponent)
	{
		// 箭已在 StartDrawBow 标记，这里实际生成并发射
		// InventoryComponent->TryRetrieveArrow(...);
	}

	// 右手回到默认位置
	RightHand->InterpToDefaultTransform();
}

// ==================== 内部函数 ====================

float ABasePCPlayer::CalculateDrawDistance() const
{
	if (!FirstPersonCamera)
	{
		return MaxDrawDistance;
	}

	// 射线检测目标位置
	FHitResult Hit;
	FVector Start = FirstPersonCamera->GetComponentLocation();
	FVector End = Start + FirstPersonCamera->GetForwardVector() * MaxShootDistance;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	float TargetDist = MaxShootDistance;
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, QueryParams))
	{
		TargetDist = FMath::Min(Hit.Distance, MaxShootDistance);
	}

	// 根据目标距离计算需要的拉弓距离
	return FMath::GetMappedRangeValueClamped(
		FVector2D(0.0f, MaxShootDistance),
		FVector2D(MinDrawDistance, MaxDrawDistance),
		TargetDist
	);
}

void ABasePCPlayer::PlayNoArrowSound()
{
	// TODO: 播放无箭音效
}
