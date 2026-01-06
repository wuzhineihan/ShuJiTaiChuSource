// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Grabber/PlayerGrabHand.h"
#include "PCGrabHand.generated.h"

class UCameraComponent;
class ABasePCPlayer;

/**
 * PC 模式手部组件
 *
 * 使用射线检测进行抓取，支持程序化手部动画。
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VRTEST_API UPCGrabHand : public UPlayerGrabHand
{
	GENERATED_BODY()

public:
	UPCGrabHand();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==================== 配置 ====================

	/** 丢弃射线最大距离 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PC|Grab")
	float MaxDropDistance = 500.0f;

	/** 抓取检测通道（用于丢弃时的射线检测） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PC|Grab")
	TEnumAsByte<ECollisionChannel> GrabTraceChannel = ECC_Visibility;

	// ==================== Interp 配置 ====================
	
	/** 手部位置插值速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PC|Interp")
	float HandInterpSpeed = 10.0f;

	/** 当前目标变换（相对于摄像机） */
	UPROPERTY(BlueprintReadOnly, Category = "PC|Interp")
	FTransform TargetRelativeTransform;

	/** 默认手部位置（相对于摄像机） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PC|Interp")
	FTransform DefaultRelativeTransform;

	/** 是否正在插值移动 */
	UPROPERTY(BlueprintReadOnly, Category = "PC|Interp")
	bool bIsInterping = false;

	// ==================== PC 专用接口 ====================
	
	/**
	 * 尝试抓取或释放（PC 徒手模式用）
	 * 根据手中是否有物体决定操作
	 */
	UFUNCTION(BlueprintCallable, Category = "PC|Grab")
	void TryGrabOrRelease();

	/**
	 * 丢弃物体到射线目标位置
	 */
	UFUNCTION(BlueprintCallable, Category = "PC|Grab")
	void DropToRaycastTarget();

	/**
	 * 程序化移动手部到目标位置
	 * @param RelativeTransform 相对于摄像机的目标变换
	 */
	UFUNCTION(BlueprintCallable, Category = "PC|Interp")
	void InterpToTransform(const FTransform& RelativeTransform);

	/**
	 * 程序化移动手部回到默认位置
	 */
	UFUNCTION(BlueprintCallable, Category = "PC|Interp")
	void InterpToDefaultTransform();

protected:
	// ==================== 重写 ====================
	
	virtual AActor* FindTarget(bool bFromBackpack, FName& OutBoneName) override;

	// ==================== 内部函数 ====================

	/** 获取所属玩家的摄像机组件 */
	UCameraComponent* GetOwnerCamera() const;

	/** 获取所属 PC 玩家 */
	ABasePCPlayer* GetOwnerPCPlayer() const;

	/** 执行射线检测（仅用于丢弃物体时） */
	bool PerformLineTrace(FHitResult& OutHit, float MaxDistance) const;

	/** 更新手部插值位置 */
	void UpdateHandInterp(float DeltaTime);
};
