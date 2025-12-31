// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Grabbee/GrabbeeWeapon.h"
#include "Bow.generated.h"

class UBoxComponent;
class UNiagaraComponent;
class UAudioComponent;
class AArrow;
class UPlayerGrabHand;
class ABaseCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnArrowFired, AArrow*, FiredArrow);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnArrowNocked, AArrow*, NockedArrow);

/**
 * 弓 - 可抓取武器
 * 
 * 不关心是VR还是PC
 * 对外仅暴露抓取接口
 * 
 * VR模式：
 * - 一只手抓弓身（BodyHeld）
 * - 另一只手抓弓弦或拿箭靠近弓弦（StringHeld）
 * - 拉弦释放发射
 * 
 * PC模式：
 * - 程序化控制弓弦拉伸
 * - 自动管理箭的生成和发射
 */
UCLASS()
class VRTEST_API ABow : public AGrabbeeWeapon
{
	GENERATED_BODY()
	
public:	
	ABow();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// ==================== 组件 ====================
	
	/** 弓弦网格体 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* StringMesh;

	/** 弓弦碰撞区域（VR 模式检测手/箭进入） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* StringCollision;

	/** 弓前端位置（用于计算箭的朝向） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* BowFrontPosition;

	/** 弓弦默认位置（未拉弦时） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* StringRestPosition;

	/** 轨迹预览 Niagara */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNiagaraComponent* TrajectoryPreview;

	/** 弓弦音效 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAudioComponent* StringAudio;

	/** 发射音效 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAudioComponent* FireAudio;

	// ==================== 配置 ====================
	
	/** 最大拉弦距离 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow|Config")
	float MaxPullDistance = 60.0f;

	/** 发射速度系数（拉弦距离 * 系数 = 发射速度） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow|Config")
	float FiringSpeedMultiplier = 50.0f;

	/** 最小发射速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow|Config")
	float MinFiringSpeed = 500.0f;

	/** 最大发射速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow|Config")
	float MaxFiringSpeed = 3000.0f;

	/** 播放弓弦音效的拉伸阈值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow|Config")
	float StringSFXThreshold = 10.0f;

	/** 弓弦弹簧强度（回弹） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow|Config")
	float StringSpringStrength = 40.0f;

	/** 弓弦弹簧阻尼 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow|Config")
	float StringSpringDamping = 0.4f;

	/** 箭类（用于生成箭） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow|Config")
	TSubclassOf<AArrow> ArrowClass;

	// ==================== 状态 ====================
	
	/** 弓身是否被抓取 */
	UPROPERTY(BlueprintReadOnly, Category = "Bow|State")
	bool bBodyHeld = false;

	/** 弓弦是否被抓取 */
	UPROPERTY(BlueprintReadOnly, Category = "Bow|State")
	bool bStringHeld = false;

	/** 抓弓身的手 */
	UPROPERTY(BlueprintReadOnly, Category = "Bow|State")
	UPlayerGrabHand* BodyHoldingHand = nullptr;

	/** 抓弓弦的手 */
	UPROPERTY(BlueprintReadOnly, Category = "Bow|State")
	UPlayerGrabHand* StringHoldingHand = nullptr;

	/** 当前搭载的箭 */
	UPROPERTY(BlueprintReadOnly, Category = "Bow|State")
	AArrow* NockedArrow = nullptr;

	/** 当前弓弦抓取点（世界坐标） */
	UPROPERTY(BlueprintReadOnly, Category = "Bow|State")
	FVector CurrentGrabSpot;

	/** 当前拉弦长度 */
	UPROPERTY(BlueprintReadOnly, Category = "Bow|State")
	float CurrentPullLength = 0.0f;

	/** 拉弦时的初始偏移（VR模式：手抓弦时的相对偏移 / PC模式：右手相对弓弦的偏移） */
	UPROPERTY(BlueprintReadOnly, Category = "Bow|State")
	FVector InitialStringGrabOffset;

	/** 手是否在弓弦碰撞区域内 */
	UPROPERTY(BlueprintReadOnly, Category = "Bow|State")
	bool bHandInStringCollision = false;

	/** 弓弦动态材质实例 */
	UPROPERTY(BlueprintReadOnly, Category = "Bow|State")
	UMaterialInstanceDynamic* StringMID = nullptr;

	/** 弓的持有者（用于伤害归属） */
	UPROPERTY(BlueprintReadOnly, Category = "Bow|State")
	ABaseCharacter* BowOwner = nullptr;

	// ==================== 委托 ====================
	
	/** 箭发射时触发 */
	UPROPERTY(BlueprintAssignable, Category = "Bow|Events")
	FOnArrowFired OnArrowFired;

	/** 箭搭上弓弦时触发 */
	UPROPERTY(BlueprintAssignable, Category = "Bow|Events")
	FOnArrowNocked OnArrowNocked;

	// ==================== 核心接口 ====================
	
	/**
	 * 停止拉弦并发射（如果有箭）
	 */
	UFUNCTION(BlueprintCallable, Category = "Bow")
	void ReleaseString();

	/**
	 * 搭箭
	 * @param Arrow 要搭载的箭
	 * @return 是否成功搭载
	 */
	UFUNCTION(BlueprintCallable, Category = "Bow")
	bool NockArrow(AArrow* Arrow);

	/**
	 * 取消搭箭（箭恢复 Idle 状态）
	 */
	UFUNCTION(BlueprintCallable, Category = "Bow")
	void UnnockArrow();

	/**
	 * 生成并搭载箭（PC模式使用）
	 * @return 生成的箭
	 */
	UFUNCTION(BlueprintCallable, Category = "Bow")
	AArrow* SpawnAndNockArrow();

	/**
	 * 发射箭
	 */
	UFUNCTION(BlueprintCallable, Category = "Bow")
	void FireArrow();

	/**
	 * 重置弓弦状态（释放但不发射）
	 */
	UFUNCTION(BlueprintCallable, Category = "Bow")
	void ResetStringState();

	/**
	 * 计算当前发射速度
	 */
	UFUNCTION(BlueprintPure, Category = "Bow")
	float CalculateFiringSpeed() const;

	/**
	 * 更新轨迹预览
	 */
	UFUNCTION(BlueprintCallable, Category = "Bow")
	void UpdateTrajectoryPreview();

	// ==================== 重写 ====================
	
	virtual EGrabType GetGrabType_Implementation() const override;
	virtual UPrimitiveComponent* GetGrabPrimitive_Implementation() const override;
	virtual bool SupportsDualHandGrab_Implementation() const override;
	virtual bool CanBeGrabbedBy_Implementation(const UPlayerGrabHand* Hand) const override;
	virtual void OnGrabbed_Implementation(UPlayerGrabHand* Hand) override;
	virtual void OnReleased_Implementation(UPlayerGrabHand* Hand) override;

protected:
	// ==================== 内部函数 ====================
	
	/**
	 * 开始拉弦（内部调用，由OnGrabbed触发）
	 * @param Hand 抓弦的手
	 */
	void StartPullingString(UPlayerGrabHand* Hand);

	/**
	 * 从碰撞组件获取对应的手部组件
	 * @param Comp 碰撞到的组件（应该是HandCollision）
	 * @return 找到的手部组件，或nullptr
	 */
	UPlayerGrabHand* GetHandFromCollision(UPrimitiveComponent* Comp) const;

	/** 弓弦碰撞开始 */
	UFUNCTION()
	void OnStringCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** 弓弦碰撞结束 */
	UFUNCTION()
	void OnStringCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** 更新弓弦位置（每帧调用） */
	void UpdateStringPosition(float DeltaTime);

	/** 更新箭的位置（每帧调用） */
	void UpdateArrowPosition();

	/** 弓弦弹簧回弹计算 */
	FVector SpringSolve(const FVector& Current, const FVector& Target, float Strength, float Damping, float DeltaTime);

	/** 弓弦回弹速度（用于弹簧计算） */
	FVector StringVelocity = FVector::ZeroVector;

	/** 是否已播放弓弦音效（防止重复播放） */
	bool bHasPlayedStringSFX = false;
};
