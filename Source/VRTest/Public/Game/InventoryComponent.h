// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class AGrabbeeObject;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnArrowCountChanged, int32, NewCount, int32, MaxCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBowStateChanged, bool, bInBackpack);

/**
 * 背包组件 - 管理角色的弓和箭
 * 
 * 设计原则：
 * - 背包只存储弓(1把)和箭(有上限)
 * - 物品是"虚拟存储"：放入时销毁Actor，取出时重新生成
 * - 提供纯数据接口，不处理VR/PC的具体交互逻辑
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VRTEST_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

protected:
	virtual void BeginPlay() override;

public:
	// ==================== 配置 ====================
	
	/** 箭矢最大数量 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 MaxArrowCount = 3;

	/** 箭的类 - 用于生成 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TSubclassOf<AGrabbeeObject> ArrowClass;

	// ==================== 状态 ====================

	/** 当前箭矢数量 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	int32 ArrowCount = 0;

	// ==================== 委托 ====================
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnArrowCountChanged OnArrowCountChanged;

	// ==================== 箭操作 ====================
	
	/**
	 * 尝试将箭放入背包
	 * @return true 如果成功放入（未达到上限）
	 */
	UFUNCTION(BlueprintCallable, Category = "Arrow")
	bool TryStoreArrow();

	/**
	 * 尝试从背包取出箭
	 * @param SpawnTransform 生成位置
	 * @return 生成的箭Actor，如果背包里没有箭则返回nullptr
	 */
	UFUNCTION(BlueprintCallable, Category = "Arrow")
	AGrabbeeObject* TryRetrieveArrow(const FTransform& SpawnTransform);

	/**
	 * 检查背包里是否有箭
	 */
	UFUNCTION(BlueprintPure, Category = "Arrow")
	bool HasArrow() const { return ArrowCount > 0; }

	/**
	 * 检查箭是否已满
	 */
	UFUNCTION(BlueprintPure, Category = "Arrow")
	bool IsArrowFull() const { return ArrowCount >= MaxArrowCount; }

	/**
	 * 获取箭数量
	 */
	UFUNCTION(BlueprintPure, Category = "Arrow")
	int32 GetArrowCount() const { return ArrowCount; }

	/**
	 * 消耗一支箭（不生成 Actor，仅减少计数）
	 * @return true 如果成功消耗
	 */
	UFUNCTION(BlueprintCallable, Category = "Arrow")
	bool ConsumeArrow();

	/**
	 * 增加一支箭（不需要 Actor，仅增加计数）
	 * @return true 如果成功增加（未达到上限）
	 */
	UFUNCTION(BlueprintCallable, Category = "Arrow")
	bool AddArrow();

	// ==================== 直接设置（用于存档/初始化） ====================

	UFUNCTION(BlueprintCallable, Category = "Arrow")
	void SetArrowCount(int32 NewCount);

	UFUNCTION(BlueprintCallable, Category = "Arrow")
	void AddArrows(int32 Amount);
};
