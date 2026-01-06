// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/BasePCPlayer.h"
#include "Grabber/PCGrabHand.h"
#include "Game/InventoryComponent.h"
#include "Grabber/IGrabbable.h"
#include "Grabbee/GrabbeeWeapon.h"
#include "Grabbee/GrabbeeObject.h"
#include "Grabbee/Bow.h"
#include "Grabbee/Arrow.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"

ABasePCPlayer::ABasePCPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建第一人称摄像机
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(RootComponent);
	FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
	FirstPersonCamera->bUsePawnControlRotation = true;

	// 创建左手
	PCLeftHand = CreateDefaultSubobject<UPCGrabHand>(TEXT("LeftHand"));
	PCLeftHand->SetupAttachment(FirstPersonCamera);
	PCLeftHand->bIsRightHand = false;
	LeftHand = PCLeftHand;  // 赋值给 BasePlayer 的基类指针

	// 创建左手碰撞体
	USphereComponent* LeftHandCollision = CreateDefaultSubobject<USphereComponent>(TEXT("LeftHandCollision"));
	LeftHandCollision->SetupAttachment(PCLeftHand);
	LeftHandCollision->SetSphereRadius(5.0f);
	LeftHandCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	LeftHandCollision->SetCollisionResponseToAllChannels(ECR_Overlap);
	LeftHandCollision->ComponentTags.Add(FName("player_hand"));
	PCLeftHand->HandCollision = LeftHandCollision;

	// 创建右手
	PCRightHand = CreateDefaultSubobject<UPCGrabHand>(TEXT("RightHand"));
	PCRightHand->SetupAttachment(FirstPersonCamera);
	PCRightHand->bIsRightHand = true;
	RightHand = PCRightHand;  // 赋值给 BasePlayer 的基类指针

	// 创建右手碰撞体
	USphereComponent* RightHandCollision = CreateDefaultSubobject<USphereComponent>(TEXT("RightHandCollision"));
	RightHandCollision->SetupAttachment(PCRightHand);
	RightHandCollision->SetSphereRadius(5.0f);
	RightHandCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RightHandCollision->SetCollisionResponseToAllChannels(ECR_Overlap);
	RightHandCollision->ComponentTags.Add(FName("player_hand"));
	PCRightHand->HandCollision = RightHandCollision;

	// 设置双手引用
	PCLeftHand->OtherHand = PCRightHand;
	PCRightHand->OtherHand = PCLeftHand;
}

void ABasePCPlayer::BeginPlay()
{
	Super::BeginPlay();

	// 绑定手的抓取/释放委托，用于同步目标检测状态
	if (PCLeftHand)
	{
		PCLeftHand->OnObjectGrabbed.AddDynamic(this, &ABasePCPlayer::OnHandGrabbedObject);
	}
	if (PCRightHand)
	{
		PCRightHand->OnObjectGrabbed.AddDynamic(this, &ABasePCPlayer::OnHandGrabbedObject);
	}
}

void ABasePCPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	// 输入绑定在蓝图中配置（Enhanced Input）
}

void ABasePCPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 每帧检测瞄准目标（只在徒手模式且未持有物体时检测）
	if (!bIsBowArmed && !PCLeftHand->bIsHolding && !PCRightHand->bIsHolding)
	{
		UpdateTargetDetection();
	}

	// 弓箭模式：拉弓时弓弦跟随 StringHoldingHand，由 Bow::Tick 处理
	// 不再需要在这里手动调用 UpdateStringFromHandPosition
}

// ==================== 重写基类 ====================

void ABasePCPlayer::SetBowArmed(bool bArmed)
{
	// 退出弓箭模式时的清理
	if (bIsBowArmed && !bArmed)
	{
		// 如果正在拉弓，直接发射（不能取消拉弓）
		if (bIsDrawingBow)
		{
			ReleaseBowString();
		}
		
		// 停止瞄准
		if (bIsAiming)
		{
			StopAiming();
		}

		// 释放左手持有的弓
		if (PCLeftHand && PCLeftHand->bIsHolding &&
		    PCLeftHand->HeldActor && IsValid(PCLeftHand->HeldActor) &&
		    PCLeftHand->HeldActor == CurrentBow)
		{
			PCLeftHand->ReleaseObject();
		}
	}

	// 调用基类处理弓的生成/销毁
	Super::SetBowArmed(bArmed);

	// 进入弓箭模式时使用 GrabHand 的抓取逻辑
	if (bArmed && CurrentBow && PCLeftHand)
	{
		PCLeftHand->GrabObject(CurrentBow);
	}
}

USceneComponent* ABasePCPlayer::GetTrackOrigin() const
{
	return FirstPersonCamera;
}

// ==================== 输入处理 ====================

void ABasePCPlayer::HandleLeftTrigger(bool bPressed)
{
	if (!bIsBowArmed)
	{
		// 徒手模式
		if (bPressed)
		{
			PCLeftHand->TryGrabOrRelease();
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
			PCRightHand->TryGrabOrRelease();
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

// ==================== 弓箭操作 ====================

void ABasePCPlayer::StartAiming()
{
	if (!bIsBowArmed || !bHasBow)
	{
		return;
	}

	bIsAiming = true;
	
	// 将左手平滑过渡到瞄准位置
	PCLeftHand->InterpToTransform(AimingLeftHandTransform);

	// 在开始瞄准时生成箭并搭在弓弦上
	if (CurrentBow)
	{
		// 检查是否有箭
		if (InventoryComponent && InventoryComponent->HasArrow())
		{
			// 消耗一支箭并生成
			InventoryComponent->ConsumeArrow();
			CurrentBow->SpawnAndNockArrow();
		}
		else
		{
			// 没有箭，播放音效提示
			PlayNoArrowSound();
		}
	}
}

void ABasePCPlayer::StopAiming()
{
	// 如果正在拉弓，直接发射（不能取消拉弓）
	if (bIsDrawingBow)
	{
		ReleaseBowString();
	}

	bIsAiming = false;
	
	// 左手回到默认位置
	PCLeftHand->InterpToDefaultTransform();

	// 清理未发射的箭（只有在没拉弓时才会有未发射的箭）
	if (CurrentBow && CurrentBow->NockedArrow)
	{
		// 退还箭到背包
		if (InventoryComponent)
		{
			InventoryComponent->AddArrow();
		}
		// 销毁生成的箭 Actor
		CurrentBow->NockedArrow->Destroy();
		CurrentBow->NockedArrow = nullptr;
	}
}

void ABasePCPlayer::StartDrawBow()
{
	if (!bIsAiming)
	{
		return;
	}

	if (!CurrentBow)
	{
		return;
	}

	// 检查是否有箭搭在弓上
	if (!CurrentBow->NockedArrow)
	{
		PlayNoArrowSound();
		return;
	}

	bIsDrawingBow = true;
	
	// 设置初始偏移：弓弦位置相对于右手的偏移
	FVector StringRestPos = CurrentBow->StringRestPosition ? 
		CurrentBow->StringRestPosition->GetComponentLocation() : 
		CurrentBow->StringMesh->GetComponentLocation();
	CurrentBow->InitialStringGrabOffset = StringRestPos - PCRightHand->GetComponentLocation();
	
	// 通过抓取系统抓弓弦（触发 OnGrabbed → StartPullingString）
	PCRightHand->GrabObject(CurrentBow);
}

void ABasePCPlayer::StopDrawBow()
{
	// DEPRECATED: 一旦开始拉弓就不能取消，松手或切换模式都会直接发射
	// 此函数保留用于兼容，但内部直接调用 ReleaseBowString
	if (bIsDrawingBow)
	{
		ReleaseBowString();
	}
}

void ABasePCPlayer::ReleaseBowString()
{
	if (!bIsDrawingBow)
	{
		return;
	}

	bIsDrawingBow = false;

	// 释放弓弦（触发 OnReleased → 发射）
	if (PCRightHand && PCRightHand->bIsHolding && PCRightHand->HeldActor == CurrentBow)
	{
		PCRightHand->ReleaseObject();
	}

	// 右手回到默认位置
	PCRightHand->InterpToDefaultTransform();
}

// ==================== 内部函数 ====================

void ABasePCPlayer::UpdateTargetDetection()
{
	// 执行射线检测
	FHitResult Hit;
	AActor* NewTarget = nullptr;
	FName NewBoneName = NAME_None;

	if (PerformLineTrace(Hit, MaxGrabDistance))
	{
		AActor* HitActor = Hit.GetActor();
		
		// 检查是否实现 IGrabbable 接口
		IGrabbable* Grabbable = Cast<IGrabbable>(HitActor);

		// 验证是否可以被抓取（检查左手或右手，取第一只空闲的手）
		if (Grabbable)
		{
			UPCGrabHand* CheckHand = !PCLeftHand->bIsHolding ? PCLeftHand : PCRightHand;
			if (!IGrabbable::Execute_CanBeGrabbedBy(HitActor, CheckHand))
			{
				NewTarget = nullptr;
			}
			else
			{
				NewTarget = HitActor;
				// 保存骨骼名（如果有）
				NewBoneName = Hit.BoneName;
			}
		}
	}

	// 检查目标是否发生变化
	if (NewTarget != TargetedObject)
	{
		AActor* OldTarget = TargetedObject;
		TargetedObject = NewTarget;
		TargetedBoneName = NewBoneName;

		// 触发委托
		OnGrabTargetChanged.Broadcast(NewTarget, OldTarget);

		// 调用物体的 OnGrabSelected / OnGrabDeselected（用于高亮等效果）
		// 添加有效性检查，防止物体已被销毁
		if (OldTarget && IsValid(OldTarget))
		{
			if (IGrabbable* OldGrabbable = Cast<IGrabbable>(OldTarget))
			{
				IGrabbable::Execute_OnGrabDeselected(OldTarget);
			}
		}
		if (NewTarget && IsValid(NewTarget))
		{
			if (IGrabbable* NewGrabbable = Cast<IGrabbable>(NewTarget))
			{
				IGrabbable::Execute_OnGrabSelected(NewTarget);
			}
		}
	}
	else
	{
		// 目标相同但骨骼名可能变化
		TargetedBoneName = NewBoneName;
	}
}

bool ABasePCPlayer::PerformLineTrace(FHitResult& OutHit, float MaxDistance) const
{
	if (!FirstPersonCamera)
	{
		return false;
	}

	FVector Start = FirstPersonCamera->GetComponentLocation();
	FVector End = Start + FirstPersonCamera->GetForwardVector() * MaxDistance;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	return GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, GrabTraceChannel, QueryParams);
}

void ABasePCPlayer::OnHandGrabbedObject(AActor* GrabbedObject)
{
	// 当任一只手抓取物体时，立即清空瞄准目标
	if (TargetedObject && IsValid(TargetedObject))
	{
		AActor* OldTarget = TargetedObject;
		TargetedObject = nullptr;
		TargetedBoneName = NAME_None;

		// 触发委托
		OnGrabTargetChanged.Broadcast(nullptr, OldTarget);

		// 取消选中状态（通过接口）
		if (IGrabbable* OldGrabbable = Cast<IGrabbable>(OldTarget))
		{
			IGrabbable::Execute_OnGrabDeselected(OldTarget);
		}
	}
}

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
