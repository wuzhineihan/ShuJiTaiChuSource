// Fill out your copyright notice in the Description page of Project Settings.

#include "Grabbee/Bow.h"
#include "Grabbee/Arrow.h"
#include "Game/PlayerGrabHand.h"
#include "Game/BaseCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ABow::ABow()
{
	PrimaryActorTick.bCanEverTick = true;

	// 设置武器类型
	WeaponType = EWeaponType::Bow;
	
	// 弓使用 WeaponSnap 类型（Attach 到手上）
	GrabType = EGrabType::WeaponSnap;

	// 创建弓弦网格体
	StringMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StringMesh"));
	StringMesh->SetupAttachment(MeshComponent);
	StringMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 创建弓弦碰撞区域
	StringCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("StringCollision"));
	StringCollision->SetupAttachment(StringMesh);
	StringCollision->SetBoxExtent(FVector(10.0f, 5.0f, 20.0f));
	StringCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	StringCollision->SetCollisionResponseToAllChannels(ECR_Overlap);
	StringCollision->OnComponentBeginOverlap.AddDynamic(this, &ABow::OnStringCollisionBeginOverlap);
	StringCollision->OnComponentEndOverlap.AddDynamic(this, &ABow::OnStringCollisionEndOverlap);

	// 创建弓前端位置标记
	BowFrontPosition = CreateDefaultSubobject<USceneComponent>(TEXT("BowFrontPosition"));
	BowFrontPosition->SetupAttachment(MeshComponent);
	BowFrontPosition->SetRelativeLocation(FVector(50.0f, 0.0f, 0.0f)); // 弓的前方

	// 创建弓弦默认位置
	StringRestPosition = CreateDefaultSubobject<USceneComponent>(TEXT("StringRestPosition"));
	StringRestPosition->SetupAttachment(MeshComponent);

	// 创建轨迹预览
	TrajectoryPreview = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrajectoryPreview"));
	TrajectoryPreview->SetupAttachment(StringMesh);
	TrajectoryPreview->SetAutoActivate(false);
	TrajectoryPreview->SetVisibility(false);

	// 创建音效组件
	StringAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("StringAudio"));
	StringAudio->SetupAttachment(MeshComponent);
	StringAudio->SetAutoActivate(false);

	FireAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("FireAudio"));
	FireAudio->SetupAttachment(MeshComponent);
	FireAudio->SetAutoActivate(false);

	// 设置默认标签
	Tags.Add(FName("Bow"));
}

void ABow::BeginPlay()
{
	Super::BeginPlay();

	// 创建弓弦动态材质
	if (StringMesh && StringMesh->GetMaterial(0))
	{
		StringMID = StringMesh->CreateAndSetMaterialInstanceDynamic(0);
	}

	// 初始化弓弦位置
	CurrentGrabSpot = StringRestPosition ? StringRestPosition->GetComponentLocation() : StringMesh->GetComponentLocation();
}

void ABow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 更新弓弦位置
	UpdateStringPosition(DeltaTime);

	// 更新箭的位置
	UpdateArrowPosition();
}

// ==================== 核心接口 ====================

void ABow::StartPullingString(UPlayerGrabHand* Hand)
{
	bStringHeld = true;
	StringHoldingHand = Hand;

	// 记录抓取时的偏移（VR模式）
	if (Hand)
	{
		InitialStringGrabOffset = StringMesh->GetComponentLocation() - Hand->GetComponentLocation();
	}

	// 显示轨迹预览
	if (NockedArrow && TrajectoryPreview)
	{
		TrajectoryPreview->SetVisibility(true);
	}
}

void ABow::ReleaseString()
{
	// 如果有箭，发射
	if (NockedArrow)
	{
		FireArrow();
	}

	bStringHeld = false;
	StringHoldingHand = nullptr;

	// 隐藏轨迹预览
	if (TrajectoryPreview)
	{
		TrajectoryPreview->SetVisibility(false);
	}

	// 重置音效状态
	bHasPlayedStringSFX = false;
}

bool ABow::NockArrow(AArrow* Arrow)
{
	if (!Arrow)
	{
		return false;
	}

	// 如果已有箭，先取消
	if (NockedArrow)
	{
		UnnockArrow();
	}

	NockedArrow = Arrow;
	NockedArrow->EnterNockedState(this);

	// 设置箭的发射者
	NockedArrow->ArrowInstigator = BowOwner;

	// 广播搭箭事件
	OnArrowNocked.Broadcast(NockedArrow);

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

AArrow* ABow::SpawnAndNockArrow()
{
	if (!ArrowClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ABow::SpawnAndNockArrow - ArrowClass not set!"));
		return nullptr;
	}

	// 在弓弦位置生成箭
	FTransform SpawnTransform;
	SpawnTransform.SetLocation(CurrentGrabSpot);
	
	// 箭朝向弓前方
	FVector Direction = BowFrontPosition->GetComponentLocation() - CurrentGrabSpot;
	SpawnTransform.SetRotation(UKismetMathLibrary::MakeRotFromX(Direction).Quaternion());

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Instigator = Cast<APawn>(BowOwner);

	AArrow* NewArrow = GetWorld()->SpawnActor<AArrow>(ArrowClass, SpawnTransform, SpawnParams);
	if (NewArrow)
	{
		NockArrow(NewArrow);
	}

	return NewArrow;
}

void ABow::FireArrow()
{
	if (!NockedArrow)
	{
		return;
	}

	// 计算发射速度
	float FiringSpeed = CalculateFiringSpeed();

	// 播放发射音效
	if (FireAudio)
	{
		FireAudio->Play();
	}

	// 发射箭
	NockedArrow->EnterFlyingState(FiringSpeed);

	// 广播发射事件
	OnArrowFired.Broadcast(NockedArrow);

	// 清除引用
	NockedArrow = nullptr;

	// 隐藏轨迹预览
	if (TrajectoryPreview)
	{
		TrajectoryPreview->SetVisibility(false);
	}
}

void ABow::ResetStringState()
{
	bStringHeld = false;
	StringHoldingHand = nullptr;
	bHasPlayedStringSFX = false;
}

float ABow::CalculateFiringSpeed() const
{
	float Speed = CurrentPullLength * FiringSpeedMultiplier;
	return FMath::Clamp(Speed, MinFiringSpeed, MaxFiringSpeed);
}

void ABow::UpdateTrajectoryPreview()
{
	if (!NockedArrow || !TrajectoryPreview)
	{
		return;
	}

	// 计算发射方向
	FVector LaunchDirection = BowFrontPosition->GetComponentLocation() - CurrentGrabSpot;
	LaunchDirection.Normalize();

	// 计算发射速度
	float Speed = CalculateFiringSpeed();
	FVector LaunchVelocity = LaunchDirection * Speed;

	// 预测轨迹
	FPredictProjectilePathParams PathParams;
	PathParams.StartLocation = CurrentGrabSpot;
	PathParams.LaunchVelocity = LaunchVelocity;
	PathParams.ProjectileRadius = 2.0f;
	PathParams.MaxSimTime = 3.0f;
	PathParams.bTraceWithCollision = true;
	PathParams.bTraceComplex = false;
	PathParams.ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	PathParams.ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody));
	PathParams.ActorsToIgnore.Add(this);
	if (NockedArrow)
	{
		PathParams.ActorsToIgnore.Add(NockedArrow);
	}

	FPredictProjectilePathResult PathResult;
	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);

	// 将轨迹点传递给 Niagara
	TArray<FVector> PathPoints;
	for (const FPredictProjectilePathPointData& PointData : PathResult.PathData)
	{
		PathPoints.Add(PointData.Location);
	}

	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(TrajectoryPreview, FName("User.PointArray"), PathPoints);
}

// ==================== 重写 ====================

EGrabType ABow::GetGrabType_Implementation() const
{
	if (!bBodyHeld)
	{
		// 弓身未被抓 → 正常武器抓取
		return EGrabType::WeaponSnap;
	}
	else
	{
		// 弓身已被抓 → 自定义处理弓弦
		return EGrabType::Custom;
	}
}

UPrimitiveComponent* ABow::GetGrabPrimitive_Implementation() const
{
	if (!bBodyHeld)
	{
		return MeshComponent;
	}
	return nullptr;  // 弓弦不需要 Primitive
}

bool ABow::SupportsDualHandGrab_Implementation() const
{
	return true;  // 弓支持双手抓取（弓身 + 弓弦）
}

bool ABow::CanBeGrabbedBy_Implementation(const UPlayerGrabHand* Hand) const
{
	if (!Hand)
	{
		return false;
	}

	// 如果弓身未被抓取，正常检查
	if (!bBodyHeld)
	{
		return bCanGrab && !bIsHeld;
	}

	// 弓身已被抓取，检查是否可以抓弓弦
	// 弓弦未被抓取 且 不是同一只手
	if (!bStringHeld && Hand != BodyHoldingHand)
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

	// 判断是抓弓身还是抓弓弦
	if (!bBodyHeld)
	{
		// 第一次抓取：抓弓身
		Super::OnGrabbed_Implementation(Hand);

		bBodyHeld = true;
		BodyHoldingHand = Hand;

		// 尝试获取弓的持有者
		AActor* HandOwner = Hand->GetOwner();
		if (ABaseCharacter* Character = Cast<ABaseCharacter>(HandOwner))
		{
			BowOwner = Character;
		}
	}
	else if (!bStringHeld && Hand != BodyHoldingHand)
	{
		// 第二次抓取：抓弓弦（必须是另一只手）
		StartPullingString(Hand);
		
		// 记录抓取时的偏移（弓弦位置相对于手部位置）
		FVector StringPos = StringRestPosition ? StringRestPosition->GetComponentLocation() : StringMesh->GetComponentLocation();
		InitialStringGrabOffset = StringPos - Hand->GetComponentLocation();
	}
}

void ABow::OnReleased_Implementation(UPlayerGrabHand* Hand)
{
	// 检查是释放弓身还是弓弦
	if (Hand == BodyHoldingHand)
	{
		// 释放弓身
		Super::OnReleased_Implementation(Hand);

		bBodyHeld = false;
		BodyHoldingHand = nullptr;

		// 同时释放弓弦
		if (bStringHeld)
		{
			ReleaseString();
		}
	}
	else if (Hand == StringHoldingHand)
	{
		// 释放弓弦（发射箭）
		ReleaseString();
	}
}

// ==================== 内部函数 ====================

UPlayerGrabHand* ABow::GetHandFromCollision(UPrimitiveComponent* Comp) const
{
	if (!Comp)
	{
		return nullptr;
	}
	
	// HandCollision 是 PlayerGrabHand 的子组件
	if (UPlayerGrabHand* Hand = Cast<UPlayerGrabHand>(Comp->GetAttachParent()))
	{
		return Hand;
	}
	
	return nullptr;
}

void ABow::OnStringCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 检查是否是玩家的手
	if (OtherComp->ComponentHasTag(FName("player_hand")))
	{
		bHandInStringCollision = true;
		
		// 获取手组件
		UPlayerGrabHand* Hand = GetHandFromCollision(OtherComp);
		if (!Hand || Hand == BodyHoldingHand)
		{
			return;
		}
		
		// 弓身必须已被抓取
		if (!bBodyHeld)
		{
			return;
		}
		
		// 弓弦已被抓取则跳过
		if (bStringHeld)
		{
			return;
		}
		
		// 检查手是否持有箭
		if (AArrow* Arrow = Cast<AArrow>(Hand->HeldActor))
		{
			// 手释放箭
			Hand->ReleaseObject();
			// 搭箭
			NockArrow(Arrow);
		}
		
		// 手抓弓弦
		Hand->GrabObject(this);
		return;
	}

	// 检查是否是箭（保留旧逻辑用于没有手持的箭）
	if (AArrow* Arrow = Cast<AArrow>(OtherActor))
	{
		// 如果箭在 Idle 状态且弓身已被抓取，自动搭箭
		if (Arrow->ArrowState == EArrowState::Idle && bBodyHeld && !NockedArrow && !Arrow->bIsHeld)
		{
			NockArrow(Arrow);
		}
	}
}

void ABow::OnStringCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// 检查是否是玩家的手离开
	if (OtherComp->ComponentHasTag(FName("player_hand")))
	{
		bHandInStringCollision = false;
	}
}

void ABow::UpdateStringPosition(float DeltaTime)
{
	FVector TargetPos;

	if (bStringHeld)
	{
		// 弓弦被拉动
		if (StringHoldingHand)
		{
			// 跟随手的位置
			TargetPos = StringHoldingHand->GetComponentLocation() + InitialStringGrabOffset;
		}
		else
		{
			// 使用 UpdateStringFromHandPosition 或 SetPullAmount 设置的位置
			TargetPos = CurrentGrabSpot;
		}

		// 限制拉弦距离
		FVector RestPos = StringRestPosition ? StringRestPosition->GetComponentLocation() : StringMesh->GetComponentLocation();
		FVector PullVector = TargetPos - RestPos;
		float PullDist = PullVector.Size();

		if (PullDist > MaxPullDistance)
		{
			TargetPos = RestPos + PullVector.GetSafeNormal() * MaxPullDistance;
			PullDist = MaxPullDistance;
		}

		CurrentGrabSpot = TargetPos;
		CurrentPullLength = PullDist;

		// 更新弓弦材质
		if (StringMID)
		{
			StringMID->SetScalarParameterValue(FName("Grabbed"), 1.0f);
			StringMID->SetVectorParameterValue(FName("GrabSpot"), FLinearColor(CurrentGrabSpot));
		}

		// 播放弓弦音效
		if (CurrentPullLength > StringSFXThreshold && !bHasPlayedStringSFX)
		{
			if (StringAudio)
			{
				StringAudio->Play();
			}
			bHasPlayedStringSFX = true;
		}

		// 更新轨迹预览
		if (NockedArrow)
		{
			UpdateTrajectoryPreview();
		}
	}
	else
	{
		// 弓弦回弹
		TargetPos = StringRestPosition ? StringRestPosition->GetComponentLocation() : StringMesh->GetComponentLocation();
		CurrentGrabSpot = SpringSolve(CurrentGrabSpot, TargetPos, StringSpringStrength, StringSpringDamping, DeltaTime);
		CurrentPullLength = (CurrentGrabSpot - TargetPos).Size();

		// 更新弓弦材质
		if (StringMID)
		{
			float GrabbedValue = CurrentPullLength > 1.0f ? 1.0f : 0.0f;
			StringMID->SetScalarParameterValue(FName("Grabbed"), GrabbedValue);
			StringMID->SetVectorParameterValue(FName("GrabSpot"), FLinearColor(CurrentGrabSpot));
		}

		bHasPlayedStringSFX = false;
	}
}

void ABow::UpdateArrowPosition()
{
	if (!NockedArrow)
	{
		return;
	}

	// 计算箭的位置和朝向
	FVector ArrowLocation = CurrentGrabSpot;
	FVector Direction = BowFrontPosition->GetComponentLocation() - CurrentGrabSpot;
	FRotator ArrowRotation = UKismetMathLibrary::MakeRotFromX(Direction);

	NockedArrow->SetActorLocationAndRotation(ArrowLocation, ArrowRotation);
}

FVector ABow::SpringSolve(const FVector& Current, const FVector& Target, float Strength, float Damping, float DeltaTime)
{
	// 弹簧公式：F = -k * x - d * v
	FVector Displacement = Current - Target;
	FVector SpringForce = -Strength * Displacement;
	FVector DampingForce = -Damping * StringVelocity;
	FVector Acceleration = SpringForce + DampingForce;

	StringVelocity += Acceleration * DeltaTime;
	FVector NewPosition = Current + StringVelocity * DeltaTime;

	// 如果足够接近目标，停止
	if (Displacement.Size() < 0.1f && StringVelocity.Size() < 0.1f)
	{
		StringVelocity = FVector::ZeroVector;
		return Target;
	}

	return NewPosition;
}
