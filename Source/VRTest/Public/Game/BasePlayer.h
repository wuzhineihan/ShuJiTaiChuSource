// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/BaseCharacter.h"
#include "Effect/FallDamageComponent.h"
#include "Effect/AutoRecoverComponent.h"
#include "BasePlayer.generated.h"

class UPhysicsControlComponent;
class AGrabbeeWeapon;
class ABow;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBowArmedChanged, bool, bIsArmed);

/**
 * 玩家基类
 * 
 * 包含所有玩家共有的组件和功能。
 * PC 和 VR 玩家各自继承此类。
 */
UCLASS()
class VRTEST_API ABasePlayer : public ABaseCharacter
{
	GENERATED_BODY()

public:
	ABasePlayer();

	// ==================== 组件 ====================
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UFallDamageComponent* FallDamageComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAutoRecoverComponent* AutoRecoverComponent;

	/** 物理控制组件 - 用于 Free/Snap/HumanBody 类型抓取 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPhysicsControlComponent* PhysicsControlComponent;

	// ==================== 弓相关 ====================
	
	/** 是否已获得弓（游戏流程中获得，永久持有） */
	UPROPERTY(BlueprintReadOnly, Category = "Bow")
	bool bHasBow = false;

	/** 是否处于弓箭模式 */
	UPROPERTY(BlueprintReadOnly, Category = "Bow")
	bool bIsBowArmed = false;

	/** 弓的类 - 用于生成 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	TSubclassOf<AGrabbeeWeapon> BowClass;

	/** 当前持有的弓（弓箭模式下不为空） */
	UPROPERTY(BlueprintReadOnly, Category = "Bow")
	ABow* CurrentBow = nullptr;

	/** 弓箭模式切换事件 */
	UPROPERTY(BlueprintAssignable, Category = "Bow|Events")
	FOnBowArmedChanged OnBowArmedChanged;

	// ==================== 弓接口 ====================
	
	/**
	 * 首次获得弓时调用（游戏流程触发）
	 * 会自动进入弓箭模式
	 */
	UFUNCTION(BlueprintCallable, Category = "Bow")
	virtual void OnBowFirstPickedUp();

	/**
	 * 切换弓箭模式
	 * @param bArmed true=进入弓箭模式，false=退出
	 */
	UFUNCTION(BlueprintCallable, Category = "Bow")
	virtual void SetBowArmed(bool bArmed);

	/**
	 * 切换弓箭模式（Toggle）
	 */
	UFUNCTION(BlueprintCallable, Category = "Bow")
	void ToggleBowArmed();

protected:
	/** 生成弓 Actor */
	virtual ABow* SpawnBow();

	/** 销毁弓 Actor */
	virtual void DestroyBow();
};
