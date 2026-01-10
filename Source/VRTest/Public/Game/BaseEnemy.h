// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/BaseCharacter.h"
#include "Grabber/IGrabbable.h"
#include "BaseEnemy.generated.h"

/**
 * 敌人基类
 * 
 * 实现 IGrabbable 接口，允许死亡后被拖拽。
 */
UCLASS()
class VRTEST_API ABaseEnemy : public ABaseCharacter, public IGrabbable
{
	GENERATED_BODY()

public:
	ABaseEnemy();
	// ==================== 状态 ====================
	
	/** 当前控制此物体的所有手（双手抓取用） */
	UPROPERTY(BlueprintReadOnly, Category = "Grab|State")
	TSet<UPlayerGrabHand*> ControllingHands;

	// ==================== IGrabbable 接口实现 ======================================
	
	virtual EGrabType GetGrabType_Implementation() const override;
	virtual UPrimitiveComponent* GetGrabPrimitive_Implementation() const override;
	virtual bool CanBeGrabbedBy_Implementation(const UPlayerGrabHand* Hand) const override;
	virtual bool CanBeGrabbedByGravityGlove_Implementation() const override;
	virtual bool SupportsDualHandGrab_Implementation() const override;
	virtual void OnGrabbed_Implementation(UPlayerGrabHand* Hand) override;
	virtual void OnReleased_Implementation(UPlayerGrabHand* Hand) override;
	virtual void OnGrabSelected_Implementation() override;
	virtual void OnGrabDeselected_Implementation() override;

	// Override OnDeath
	virtual void OnDeath_Implementation() override;
};
