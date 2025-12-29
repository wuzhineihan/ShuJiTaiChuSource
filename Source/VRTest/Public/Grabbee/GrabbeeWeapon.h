// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Grabbee/GrabbeeObject.h"
#include "GrabbeeWeapon.generated.h"

/**
 * 武器基类
 * 
 * 继承自 GrabbeeObject，用于弓、箭等武器。
 * 默认使用 WeaponSnap 抓取类型（Attach 到手上）。
 */
UCLASS()
class VRTEST_API AGrabbeeWeapon : public AGrabbeeObject
{
	GENERATED_BODY()
	
public:	
	AGrabbeeWeapon();

	// ==================== 武器配置 ====================
	
	/** 武器类型 - 用于确定抓取偏移 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	EWeaponType WeaponType = EWeaponType::None;

	// ==================== 重写 ====================
	
	virtual void OnGrabbed_Implementation(UPlayerGrabHand* Hand) override;
	virtual void OnReleased_Implementation(UPlayerGrabHand* Hand) override;
};
