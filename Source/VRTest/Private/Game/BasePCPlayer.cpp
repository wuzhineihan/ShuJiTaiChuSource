// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/BasePCPlayer.h"
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
#include "Skill/Stasis/IStasisable.h"
#include "Tools/GameUtils.h"

ABasePCPlayer::ABasePCPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建第一人称摄像机
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(RootComponent);
	FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
	FirstPersonCamera->bUsePawnControlRotation = true;
	PlayerCamera = FirstPersonCamera;

	// 创建左手
	PCLeftHand = CreateDefaultSubobject<UPCGrabHand>(TEXT("LeftHand"));
	PCLeftHand->SetupAttachment(FirstPersonCamera);
	PCLeftHand->bIsRightHand = false;
	LeftHand = PCLeftHand;  // 赋值给 BasePlayer 的基类指针

	// 创建左手碰撞体
	LeftHandCollision = CreateDefaultSubobject<USphereComponent>(TEXT("LeftHandCollision"));
	LeftHandCollision->SetupAttachment(PCLeftHand);
	LeftHandCollision->SetSphereRadius(5.0f);
	LeftHandCollision->SetCollisionProfileName(FName("Profile_PlayerHand"));
	LeftHandCollision->SetGenerateOverlapEvents(true);
	PCLeftHand->HandCollision = LeftHandCollision;

	// 创建右手
	PCRightHand = CreateDefaultSubobject<UPCGrabHand>(TEXT("RightHand"));
	PCRightHand->SetupAttachment(FirstPersonCamera);
	PCRightHand->bIsRightHand = true;
	RightHand = PCRightHand;  // 赋值给 BasePlayer 的基类指针

	// 创建右手碰撞体
	RightHandCollision = CreateDefaultSubobject<USphereComponent>(TEXT("RightHandCollision"));
	RightHandCollision->SetupAttachment(PCRightHand);
	RightHandCollision->SetSphereRadius(5.0f);
	RightHandCollision->SetCollisionProfileName(FName("Profile_PlayerHand"));
	RightHandCollision->SetGenerateOverlapEvents(true);
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

	if (!bIsBowArmed)
	{
		UpdateTargetDetection();
	}
	
}

// ==================== 重写基类 ====================

void ABasePCPlayer::SetBowArmed(bool bArmed)
{
	// 退出弓箭模式时的 PC 特有清理
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
	}
	
	Super::SetBowArmed(bArmed);
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

void ABasePCPlayer::TryThrow(bool bRightHand)
{
	if (bIsBowArmed)
		return;
	
	UPCGrabHand* ThrowHand = bRightHand ? PCRightHand : PCLeftHand;
	if (!ThrowHand || !FirstPersonCamera)
	{
		return;
	}

	// 手里没东西就返回
	if (!ThrowHand->bIsHolding || !ThrowHand->HeldActor)
	{
		return;
	}

	// 特殊处理：StasisPoint 投掷
	if (AStasisPoint* StasisPoint = Cast<AStasisPoint>(ThrowHand->HeldActor))
	{
		HandleStasisPointThrow(ThrowHand, StasisPoint);
		return;
	}

	// 只有 GrabbeeObject 才允许投掷
	AGrabbeeObject* ThrowObject = Cast<AGrabbeeObject>(ThrowHand->HeldActor);
	if (!ThrowObject)
	{
		return;
	}

	// 通过射线计算投掷目标点（从摄像机朝前）
	FHitResult Hit;
	const bool bHit = PerformLineTrace(Hit, MaxThrowDistance, ECC_GameTraceChannel3);

	const FVector Start = FirstPersonCamera->GetComponentLocation();
	const FVector End = Start + FirstPersonCamera->GetForwardVector() * MaxThrowDistance;
	const FVector TargetPoint = bHit ? Hit.ImpactPoint : End;

	// 先释放（解除 PhysicsHandle / 附着），再发射
	ThrowHand->ReleaseObject();

	// LaunchTowards 内部会清速度并加冲量
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

	// 1. 查找定身球目标（使用 GameUtils 工具函数）
	FVector CameraLocation = FirstPersonCamera->GetComponentLocation();
	FVector CameraForward = FirstPersonCamera->GetForwardVector();

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);
	IgnoreActors.Add(StasisPoint);

	// 使用与 VR 重力手套相同的检测对象类型
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	// 查找锥形范围内的所有 Actor
	TArray<FActorWithAngle> ActorsInCone = UGameUtils::FindActorsInCone(
		this,
		CameraLocation,
		CameraForward,
		StasisDetectionRadius,
		StasisDetectionAngle,
		ObjectTypes,
		IgnoreActors
	);

	// 筛选出实现了 IStasisable 接口且允许进入定身的目标（取角度最小的）
	AActor* TargetActor = nullptr;
	for (const FActorWithAngle& Item : ActorsInCone)
	{
		AActor* Actor = Item.Actor;
		if (!Actor)
		{
			continue;
		}

		if (!Actor->GetClass()->ImplementsInterface(UStasisable::StaticClass()))
		{
			continue;
		}

		// 只允许可进入定身的目标成为锁定目标
		if (!IStasisable::Execute_CanEnterStasis(Actor))
		{
			continue;
		}

		TargetActor = Actor;
		break; // 已按角度排序，找到第一个即可
	}

	// 2. 计算发射速度
	FVector InitVelocity = CameraForward * StasisFireSpeedScalar;

	// 3. 释放定身球（解除抓取）
	ThrowHand->ReleaseObject();

	// 4. 发射定身球
	StasisPoint->Fire(InitVelocity, TargetActor);

	// 5. 解锁手部
	ThrowHand->SetGrabLock(false);
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

	// 清理未发射的箭
	// 情况1：箭还在右手中（未开始拉弓）
	AArrow* HeldArrow = Cast<AArrow>(PCRightHand->HeldActor);
	if (HeldArrow)
	{
		PCRightHand->ReleaseObject();
		if (InventoryComponent)
		{
			InventoryComponent->TryStoreArrow();
		}
		HeldArrow->Destroy();
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

	// 检查库存是否有箭
	if (!InventoryComponent || !InventoryComponent->HasArrow())
	{
		PlayNoArrowSound();
		return;
	}

	// 从库存取出箭
	FTransform SpawnTransform;
	SpawnTransform.SetLocation(PCRightHand->GetComponentLocation());
	SpawnTransform.SetRotation(PCRightHand->GetComponentRotation().Quaternion());
	
	AGrabbeeObject* ArrowActor = InventoryComponent->TryRetrieveArrow(SpawnTransform);
	if (!ArrowActor)
	{
		PlayNoArrowSound();
		return;
	}

	// 让右手抓住箭
	PCRightHand->GrabObject(ArrowActor);

	bIsDrawingBow = true;
	
	// 计算弓弦位置
	FVector StringRestPos = CurrentBow->StringRestPosition ? 
		CurrentBow->StringRestPosition->GetComponentLocation() : 
		CurrentBow->StringMesh->GetComponentLocation();
	
	// 将右手移动到弓弦位置（保持现有逻辑：先把手放到弦附近，确保搭箭/抓弦逻辑能复用）
	FTransform StringTransform;
	StringTransform.SetLocation(StringRestPos);
	StringTransform.SetRotation(CurrentBow->GetActorRotation().Quaternion());
	PCRightHand->SetWorldTransform(StringTransform);

	// PC 模式：如果右手此时已经在弓弦碰撞区域内，BeginOverlap 不会再次触发。
	// 主动调用 Bow 的接口复用 OnStringCollisionBeginOverlap 的搭箭/抓弦逻辑。
	if (CurrentBow)
	{
		CurrentBow->TryHandleStringHandEnter(PCRightHand);
	}

	// PC 简化方案：固定拉弓
	// 用“摄像机前向的反方向”把右手拉到一个固定距离（相对摄像机坐标系），
	// 这样 Bow::UpdateStringPosition 会自然产生 CurrentPullLength，从而发射速度由 Bow 统一计算。
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

	bTraceHit = PerformLineTrace(Hit, MaxGrabDistance, GrabTraceChannel);
	
	if (bTraceHit)
	{
		// 保存射线碰撞点位置
		TraceTargetLocation = Hit.Location;
		
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
	else
	{
		// 没有命中任何目标
		TraceTargetLocation = FVector::ZeroVector;
	}

	// 检查目标是否发生变化
	if (NewTarget != TargetedObject)
	{
		AActor* OldTarget = TargetedObject;
		TargetedObject = NewTarget;
		TargetedBoneName = NewBoneName;

		// 调用物体的 OnGrabSelected / OnGrabDeselected（用于高亮等效果）
		// 添加有效性检查，防止物体已被销毁
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
		// 目标相同但骨骼名可能变化
		TargetedBoneName = NewBoneName;
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
	// 当任一只手抓取物体时，立即清空瞄准目标
	if (TargetedObject && IsValid(TargetedObject))
	{
		AActor* OldTarget = TargetedObject;
		TargetedObject = nullptr;
		TargetedBoneName = NAME_None;

		// 取消选中状态（通过接口）
		if (Cast<IGrabbable>(OldTarget))
		{
			IGrabbable::Execute_OnGrabDeselected(OldTarget);
		}
	}
}


void ABasePCPlayer::PlayNoArrowSound()
{
	// TODO: 播放无箭音效
}
