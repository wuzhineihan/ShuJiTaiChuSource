// Fill out your copyright notice in the Description page of Project Settings.

#include "Grabbee/Arrow.h"
#include "Grabbee/Bow.h"
#include "Grabber/PlayerGrabHand.h"
#include "Game/BaseCharacter.h"
#include "Components/BoxComponent.h"
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
	
	// 箭使用 Free 抓取类型（物理跟随）
	GrabType = EGrabType::Free;

	// 创建箭头碰撞盒
	ArrowTipCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("ArrowTipCollision"));
	ArrowTipCollision->SetupAttachment(MeshComponent);
	ArrowTipCollision->SetBoxExtent(FVector(5.0f, 2.0f, 2.0f));
	ArrowTipCollision->SetRelativeLocation(FVector(30.0f, 0.0f, 0.0f)); // 箭头位置
	ArrowTipCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ArrowTipCollision->SetCollisionResponseToAllChannels(ECR_Overlap);
	ArrowTipCollision->OnComponentBeginOverlap.AddDynamic(this, &AArrow::OnArrowTipBeginOverlap);

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

	// Nocked 状态下跟随弓弦位置（由 Bow 更新）
}

// ==================== 状态切换 ====================

void AArrow::EnterIdleState()
{
	ArrowState = EArrowState::Idle;
	
	// 启用物理模拟
	if (MeshComponent)
	{
		MeshComponent->SetSimulatePhysics(true);
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
	}

	// 禁用箭头碰撞（只在飞行时启用）
	if (ArrowTipCollision)
	{
		ArrowTipCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 禁用箭头碰撞
	if (ArrowTipCollision)
	{
		ArrowTipCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
	ArrowState = EArrowState::Flying;

	// 禁用物理模拟（由 ProjectileMovement 控制）
	if (MeshComponent)
	{
		MeshComponent->SetSimulatePhysics(false);
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}

	// 启用箭头碰撞检测
	if (ArrowTipCollision)
	{
		ArrowTipCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}

	// 启用投射物移动
	if (ProjectileMovement)
	{
		ProjectileMovement->SetActive(true);
		ProjectileMovement->bRotationFollowsVelocity = true;
		// 设置初始速度（沿箭的前方向）
		FVector LaunchDirection = GetActorForwardVector();
		ProjectileMovement->Velocity = LaunchDirection * LaunchSpeed;
	}

	// 启用轨迹效果
	if (TrailEffect)
	{
		TrailEffect->SetActive(true);
	}

	// 清除弓引用
	NockedBow = nullptr;
	bCanGrab = false;
}

void AArrow::EnterStuckState(USceneComponent* HitComponent, FName BoneName)
{
	ArrowState = EArrowState::Stuck;

	// 停止投射物移动
	if (ProjectileMovement)
	{
		ProjectileMovement->SetActive(false);
	}

	// 禁用碰撞
	if (MeshComponent)
	{
		MeshComponent->SetSimulatePhysics(false);
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (ArrowTipCollision)
	{
		ArrowTipCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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

	bCanGrab = false;
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

bool AArrow::CanBeGrabbedBy_Implementation(const UPlayerGrabHand* Hand) const
{
	// 只有在 Idle 状态下才能被抓取
	return Super::CanBeGrabbedBy_Implementation(Hand) && ArrowState == EArrowState::Idle;
}

void AArrow::OnGrabbed_Implementation(UPlayerGrabHand* Hand)
{
	Super::OnGrabbed_Implementation(Hand);

	// 被抓取时确保进入 Idle 状态（从 Stuck 状态恢复时）
	if (ArrowState == EArrowState::Stuck)
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		EnterIdleState();
	}
}

void AArrow::OnReleased_Implementation(UPlayerGrabHand* Hand)
{
	Super::OnReleased_Implementation(Hand);

	// 释放后保持 Idle 状态
	// 如果是在 Nocked 状态下释放，由 Bow 处理
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

void AArrow::OnArrowTipBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 只在飞行状态下处理碰撞
	if (ArrowState != EArrowState::Flying)
	{
		return;
	}

	// 忽略自己和弓
	if (OtherActor == this || OtherActor->ActorHasTag(FName("Bow")))
	{
		return;
	}

	// 忽略发射者
	if (ArrowInstigator && OtherActor == ArrowInstigator)
	{
		return;
	}

	// 防止重复命中
	if (bHasHit)
	{
		return;
	}

	bHasHit = true;

	// 造成伤害
	DealDamage(OtherActor);

	// 获取命中骨骼名称（如果是骨骼网格体）
	FName BoneName = NAME_None;
	if (USkeletalMeshComponent* SkelMesh = Cast<USkeletalMeshComponent>(OtherComp))
	{
		BoneName = SkelMesh->GetBoneName(OtherBodyIndex);
		// 获取父骨骼以获得更稳定的附着点
		BoneName = SkelMesh->GetParentBone(BoneName);
	}

	// 进入插入状态
	EnterStuckState(OtherComp, BoneName);

	// 一段时间后销毁（可选）
	// SetLifeSpan(10.0f);
}

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
		Effect.Instigator = ArrowInstigator;

		// 如果箭着火，添加火焰效果
		if (bOnFire)
		{
			Effect.EffectTypes.Add(EEffectType::Fire);
		}

		IEffectable::Execute_ApplyEffect(HitActor, Effect);
	}
}
