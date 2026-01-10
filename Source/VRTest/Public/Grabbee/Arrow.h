// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Grabbee/GrabbeeWeapon.h"
#include "Effect/Effectable.h"
#include "Arrow.generated.h"

class UProjectileMovementComponent;
class UNiagaraComponent;
class ABow;

/**
 * 箭 - 可抓取武器
 * 
 * 状态机：
 * - Idle: 闲置状态，可被抓取，启用物理
 * - Nocked: 搭在弓弦上，禁用物理，跟随弓弦位置
 * - Flying: 飞行中，使用 ProjectileMovement
 * - Stuck: 插在目标上
 * 
 * VR模式：玩家抓取箭 → 靠近弓弦 → 搭箭 → 拉弦 → 释放发射
 * PC模式：进入弓箭模式 → 自动生成箭 → 程序化控制
 */
UCLASS()
class VRTEST_API AArrow : public AGrabbeeWeapon, public IEffectable
{
	GENERATED_BODY()
	
public:	
	AArrow();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// ==================== 组件 ====================
	
	/** 投射物移动组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;

	/** 箭头位置（用于 LineTrace 检测） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* ArrowTipPosition;

	/** 飞行轨迹 Niagara 效果 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNiagaraComponent* TrailEffect;

	/** 火焰粒子效果 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UParticleSystemComponent* FireEffect;

	// ==================== 配置 ====================
	
	/** 箭伤害值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow|Combat")
	float ArrowDamage = 50.0f;

	/** 着火持续时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow|Combat")
	float OnFireDuration = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow|Combat")
	float ImpulseStrengthMultiplier = 2.0f;

	// ==================== 状态 ====================
	
	/** 当前箭的状态 */
	UPROPERTY(BlueprintReadOnly, Category = "Arrow|State")
	EArrowState ArrowState = EArrowState::Idle;

	/** 是否已命中目标 */
	UPROPERTY(BlueprintReadOnly, Category = "Arrow|State")
	bool bHasHit = false;

	/** 命中的骨骼名称（如果命中角色） */
	UPROPERTY(BlueprintReadOnly, Category = "Arrow|State")
	FName HitBoneName;

	/** 是否着火 */
	UPROPERTY(BlueprintReadOnly, Category = "Arrow|State")
	bool bOnFire = false;

	/** 当前搭载此箭的弓（Nocked 状态时有效） */
	UPROPERTY(BlueprintReadOnly, Category = "Arrow|State")
	ABow* NockedBow = nullptr;

	// OwningCharacter 继承自父类 GrabbeeObject，用于伤害归属

	// ==================== 状态切换 ====================
	
	/** 进入闲置状态 - 可被抓取，启用物理 */
	UFUNCTION(BlueprintCallable, Category = "Arrow|State")
	void EnterIdleState();

	/** 进入搭弦状态 - 禁用物理和碰撞，跟随弓弦 */
	UFUNCTION(BlueprintCallable, Category = "Arrow|State")
	void EnterNockedState(ABow* Bow);

	/** 进入飞行状态 - 启用 ProjectileMovement */
	UFUNCTION(BlueprintCallable, Category = "Arrow|State")
	void EnterFlyingState(float LaunchSpeed);

	/** 进入插入状态 - 停止运动，附着到目标 */
	UFUNCTION(BlueprintCallable, Category = "Arrow|State")
	void EnterStuckState(USceneComponent* HitComponent, FName BoneName);

	// ==================== 火焰效果 ====================
	
	/** 点燃箭 */
	UFUNCTION(BlueprintCallable, Category = "Arrow|Fire")
	void CatchFire();

	/** 熄灭箭 */
	UFUNCTION(BlueprintCallable, Category = "Arrow|Fire")
	void Extinguish();

	// ==================== 重写 ====================

	virtual void OnGrabbed_Implementation(UPlayerGrabHand* Hand) override;
	virtual void OnReleased_Implementation(UPlayerGrabHand* Hand) override;

	// ==================== IEffectable 接口 ====================
	
	virtual void ApplyEffect_Implementation(const FEffect& Effect) override;

protected:
	// ==================== 内部函数 ====================
	
	/** 飞行时执行 LineTrace 检测碰撞 */
	void PerformFlightTrace(float DeltaTime);

	/** 处理命中 */
	void HandleHit(const FHitResult& HitResult);

	/** 造成伤害 */
	void DealDamage(AActor* HitActor);

	/** 火焰计时器回调 */
	void OnFireTimerExpired();

	/** 火焰计时器句柄 */
	FTimerHandle FireTimerHandle;

	/** 上一帧箭头位置（用于 LineTrace） */
	FVector PreviousTipLocation;
};
