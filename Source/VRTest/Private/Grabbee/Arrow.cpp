// Fill out your copyright notice in the Description page of Project Settings.

#include "Grabbee/Arrow.h"
#include "Grabbee/Bow.h"
#include "Grabber/PlayerGrabHand.h"
#include "Game/BaseCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

AArrow::AArrow()
{
	PrimaryActorTick.bCanEverTick = true;

	// 设置武器类型
	WeaponType = EWeaponType::Arrow;
	GrabType = EGrabType::WeaponSnap;

	// 创建箭头位置标记（用于 LineTrace）
	ArrowTipPosition = CreateDefaultSubobject<USceneComponent>(TEXT("ArrowTipPosition"));
	ArrowTipPosition->SetupAttachment(MeshComponent);
	ArrowTipPosition->SetRelativeLocation(FVector(30.0f, 0.0f, 0.0f)); // 箭头位置

	// 创建投射物移动组件
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = MeshComponent;
	ProjectileMovement->InitialSpeed = 0.0f;
	ProjectileMovement->MaxSpeed = 5000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 1.0f;
	ProjectileMovement->SetActive(false);

	// 创建轨迹效果（蓝图中配置 Niagara 系统）
	TrailEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailEffect"));
	TrailEffect->SetupAttachment(MeshComponent);
	TrailEffect->SetAutoActivate(false);

	// 创建火焰粒子效果
	FireEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FireEffect"));
	FireEffect->SetupAttachment(MeshComponent);
	FireEffect->SetAutoActivate(false);
	FireEffect->SetVisibility(false);

	// 设置默认标签（用于弓识别箭）
	Tags.Add(FName("Arrow"));
}

void AArrow::BeginPlay()
{
	Super::BeginPlay();
	
	// 默认进入闲置状态
	EnterIdleState();
}

void AArrow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 飞行状态：使用 LineTrace 检测碰撞
	if (ArrowState == EArrowState::Flying && ProjectileMovement && ProjectileMovement->IsActive())
	{
		PerformFlightTrace(DeltaTime);
	}
}

// ==================== 状态切换 ====================

void AArrow::EnterIdleState()
{
	ArrowState = EArrowState::Idle;
	
	// 启用物理模拟
	if (MeshComponent)
	{
		// 确保先 Detach，避免在 Attached 状态下启用物理导致冲突
		if (GetAttachParentActor())
		{
			DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		}
		
		MeshComponent->SetSimulatePhysics(true);
		MeshComponent->SetCollisionProfileName("IgnoreOnlyPawn");
	}


	// 禁用投射物移动
	if (ProjectileMovement)
	{
		ProjectileMovement->SetActive(false);
	}

	// 禁用轨迹效果
	if (TrailEffect)
	{
		TrailEffect->SetActive(false);
	}

	// 重置命中状态
	bHasHit = false;
	NockedBow = nullptr;
	bCanGrab = true;
}

void AArrow::EnterNockedState(ABow* Bow)
{
	if (!Bow)
	{
		return;
	}

	ArrowState = EArrowState::Nocked;
	NockedBow = Bow;

	// 禁用物理和碰撞（由弓控制位置）
	if (MeshComponent)
	{
		MeshComponent->SetSimulatePhysics(false);
		MeshComponent->SetCollisionProfileName("NoCollision");
	}


	// 禁用投射物移动
	if (ProjectileMovement)
	{
		ProjectileMovement->SetActive(false);
	}

	bCanGrab = false;
}

void AArrow::EnterFlyingState(float LaunchSpeed)
{
	// 检查是否 attach 到其他 Actor
	AActor* AttachParent = GetAttachParentActor();
	
	// 如果 attach 了，先 detach
	if (AttachParent)
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
	
	ArrowState = EArrowState::Flying;

	// 禁用物理模拟（由 ProjectileMovement 控制）
	if (MeshComponent)
	{
		MeshComponent->SetSimulatePhysics(false);
		// 先完全禁用碰撞，让箭能飞起来
		MeshComponent->SetCollisionProfileName("NoCollision");
		MeshComponent->SetMobility(EComponentMobility::Movable);
	}


	// 启用投射物移动
	if (ProjectileMovement)
	{
		// 1. 先设置 UpdatedComponent
		ProjectileMovement->SetUpdatedComponent(MeshComponent);
		
		// 2. 配置参数
		ProjectileMovement->InitialSpeed = LaunchSpeed;
		ProjectileMovement->MaxSpeed = LaunchSpeed * 2.0f;
		ProjectileMovement->bRotationFollowsVelocity = true;
		ProjectileMovement->ProjectileGravityScale = 1.0f;
		
		// 3. 激活组件
		ProjectileMovement->Activate(true);
		ProjectileMovement->SetComponentTickEnabled(true);
		
		// 4. 最后设置速度（使用 SetVelocityInLocalSpace 强制设置）
		ProjectileMovement->SetVelocityInLocalSpace(FVector(LaunchSpeed, 0, 0));
	}

	// 启用轨迹效果
	if (TrailEffect)
	{
		TrailEffect->SetActive(true);
	}

	// 初始化上一帧箭头位置（用于 LineTrace）
	PreviousTipLocation = ArrowTipPosition ? ArrowTipPosition->GetComponentLocation() : GetActorLocation();

	// 清除弓引用
	NockedBow = nullptr;
	bCanGrab = false;
}

void AArrow::EnterStuckState(USceneComponent* HitComponent, FName BoneName)
{
	UE_LOG(LogTemp, Warning, TEXT("[ArrowGrab] EnterStuckState"));
	
	ArrowState = EArrowState::Stuck;

	// 停止投射物移动
	if (ProjectileMovement)
	{
		ProjectileMovement->SetActive(false);
	}

	// 禁用物理模拟但保留 Query 碰撞（这样 GrabHand 的 SphereTrace 才能检测到）
	if (MeshComponent)
	{
		MeshComponent->SetSimulatePhysics(false);
		MeshComponent->SetCollisionProfileName("OverlapAllDynamic");
	}

	// 禁用轨迹效果
	if (TrailEffect)
	{
		TrailEffect->SetActive(false);
	}

	// 附着到命中组件
	if (HitComponent)
	{
		HitBoneName = BoneName;
		AttachToComponent(HitComponent, FAttachmentTransformRules::KeepWorldTransform, BoneName);
	}

	bCanGrab = true;
}

// ==================== 火焰效果 ====================

void AArrow::CatchFire()
{
	if (bOnFire)
	{
		// 如果已经着火，重置计时器
		GetWorldTimerManager().ClearTimer(FireTimerHandle);
	}

	bOnFire = true;

	// 显示火焰效果
	if (FireEffect)
	{
		FireEffect->SetVisibility(true);
		FireEffect->Activate(true);
	}

	// 设置熄灭计时器
	GetWorldTimerManager().SetTimer(FireTimerHandle, this, &AArrow::OnFireTimerExpired, OnFireDuration, false);
}

void AArrow::Extinguish()
{
	bOnFire = false;

	// 隐藏火焰效果
	if (FireEffect)
	{
		FireEffect->SetVisibility(false);
		FireEffect->Deactivate();
	}

	// 清除计时器
	GetWorldTimerManager().ClearTimer(FireTimerHandle);
}

void AArrow::OnFireTimerExpired()
{
	Extinguish();
}

// ==================== 重写 ====================

bool AArrow::CanBeGrabbedByGravityGlove_Implementation() const
{
	// 只有处于 Idle 状态才允许重力手套抓取
	return ArrowState == EArrowState::Idle;
}

void AArrow::OnGrabbed_Implementation(UPlayerGrabHand* Hand)
{
	UE_LOG(LogTemp, Warning, TEXT("[ArrowGrab] OnGrabbed - State: %d"), static_cast<int32>(ArrowState));
	if (ArrowState== EArrowState::Stuck)
	{
		EnterIdleState();
	}
	
	// 调用父类处理抓取逻辑
	Super::OnGrabbed_Implementation(Hand);
}

void AArrow::OnReleased_Implementation(UPlayerGrabHand* Hand)
{
	Super::OnReleased_Implementation(Hand);

	// 释放后保持 Idle 状态
	// 如果是在 Nocked 状态下释放，由 Bow 处理
}

// ==================== 飞行检测 ====================

void AArrow::PerformFlightTrace(float DeltaTime)
{
	if (bHasHit)
	{
		return;
	}

	// 获取当前箭头位置
	FVector CurrentTipLocation = ArrowTipPosition ? ArrowTipPosition->GetComponentLocation() : GetActorLocation();

	// 执行 LineTrace
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	
	// 忽略发射者（玩家）
	if (OwningCharacter)
	{
		QueryParams.AddIgnoredActor(OwningCharacter);
		
		// 如果发射者有Owner（比如玩家控制器），也忽略
		AActor* InstigatorOwner = OwningCharacter->GetOwner();
		if (InstigatorOwner)
		{
			QueryParams.AddIgnoredActor(InstigatorOwner);
		}
	}

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		PreviousTipLocation,
		CurrentTipLocation,
		ECC_Visibility,
		QueryParams
	);

	if (bHit && HitResult.GetActor())
	{
		// 忽略弓
		if (!HitResult.GetActor()->ActorHasTag(FName("Bow")))
		{
			HandleHit(HitResult);
		}
	}

	// 更新上一帧位置
	PreviousTipLocation = CurrentTipLocation;
}

void AArrow::HandleHit(const FHitResult& HitResult)
{
	if (bHasHit)
	{
		return;
	}

	bHasHit = true;

	AActor* HitActor = HitResult.GetActor();
	UPrimitiveComponent* HitComp = HitResult.GetComponent();

	// 造成伤害
	DealDamage(HitActor);

	// 获取命中骨骼名称（如果是骨骼网格体）
	FName BoneName = HitResult.BoneName;

	// 准备物理冲量数据
	FVector ImpulseDir = FVector::ZeroVector;
	float ImpulseStrength = 0.0f;
	bool bShouldApplyImpulse = false;

	if (HitComp && HitComp->IsSimulatingPhysics())
	{
		bShouldApplyImpulse = true;
		ImpulseDir = GetActorForwardVector();
		
		if (ProjectileMovement && ProjectileMovement->Velocity.SizeSquared() > 1.0f)
		{
			ImpulseDir = ProjectileMovement->Velocity.GetSafeNormal();
			// 动量 = 质量 * 速度，这里简单模拟
			ImpulseStrength = ProjectileMovement->Velocity.Size() * ImpulseStrengthMultiplier; 
		}
	}

	// 将箭移动到命中点（根据箭头位置组件的相对偏移）
	float TipOffset = ArrowTipPosition ? ArrowTipPosition->GetRelativeLocation().X : 30.0f;
	SetActorLocation(HitResult.ImpactPoint - GetActorForwardVector() * TipOffset);

	// 进入插入状态
	EnterStuckState(HitComp, BoneName);

	// 在附着后施加物理冲量，确保物体带着箭一起受到影响
	if (bShouldApplyImpulse && HitComp)
	{
		HitComp->AddImpulseAtLocation(ImpulseDir * ImpulseStrength, HitResult.ImpactPoint, HitResult.BoneName);
	}
}

// ==================== IEffectable 接口 ====================

void AArrow::ApplyEffect_Implementation(const FEffect& Effect)
{
	// 检查是否有火焰效果
	for (EEffectType EffectType : Effect.EffectTypes)
	{
		if (EffectType == EEffectType::Fire)
		{
			CatchFire();
			break;
		}
	}
}

// ==================== 内部函数 ====================


void AArrow::DealDamage(AActor* HitActor)
{
	if (!HitActor)
	{
		return;
	}

	// 检查目标是否实现 IEffectable 接口
	if (HitActor->Implements<UEffectable>())
	{
		FEffect Effect;
		Effect.EffectTypes.Add(EEffectType::Arrow);
		Effect.Amount = ArrowDamage;
		Effect.Causer = this;
		Effect.Instigator = OwningCharacter;

		// 如果箭着火，添加火焰效果
		if (bOnFire)
		{
			Effect.EffectTypes.Add(EEffectType::Fire);
		}

		IEffectable::Execute_ApplyEffect(HitActor, Effect);
	}
}
