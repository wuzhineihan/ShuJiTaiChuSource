// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/Characters/BaseCharacter.h"
#include "Effect/FallDamageComponent.h"
#include "Effect/AutoRecoverComponent.h"
#include "Game/InventoryComponent.h"
#include "BasePlayer.generated.h"

class UCameraComponent;
class UPhysicsHandleComponent;
class UPlayerGrabHand;
class ABow;
class UPlayerSkillComponent;
class UPlayerClimbComponent;
class AActor;

/**
 * 玩家基类
 * 
 * 包含所有玩家共有的组件和功能�?
 * PC �?VR 玩家各自继承此类�?
 */
UCLASS()
class VRTEST_API ABasePlayer : public ABaseCharacter
{
	GENERATED_BODY()

public:
	ABasePlayer();

protected:
	virtual void BeginPlay() override;

public:
	// ==================== 组件 ====================
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UFallDamageComponent* FallDamageComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAutoRecoverComponent* AutoRecoverComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInventoryComponent* InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPlayerSkillComponent* PlayerSkillComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPlayerClimbComponent* PlayerClimbComponent;

	/** 左手物理抓取组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPhysicsHandleComponent* LeftPhysicsHandle;

	/** 右手物理抓取组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPhysicsHandleComponent* RightPhysicsHandle;

	/** 左手抓取组件（基类指针，子类创建具体类型�?*/
	UPROPERTY(BlueprintReadOnly, Category = "Components")
	UPlayerGrabHand* LeftHand = nullptr;

	/** 右手抓取组件（基类指针，子类创建具体类型�?*/
	UPROPERTY(BlueprintReadOnly, Category = "Components")
	UPlayerGrabHand* RightHand = nullptr;
	
	UPROPERTY(BlueprintReadOnly, Category = "Components")
	UCameraComponent* PlayerCamera;
	
	
	virtual USceneComponent* GetTrackOrigin() const override;

	// ==================== 弓接�?====================
	
	/**
	 * 首次获得弓时调用（游戏流程触发）
	 * 会自动进入弓箭模�?
	 * @return true=首次获得弓，false=已获得过�?
	 */
	UFUNCTION(BlueprintCallable, Category = "Bow")
	virtual bool CheckBowFirstPickedUp();

	/**
	 * 切换弓箭模式
	 * @param bArmed true=进入弓箭模式，false=退�?
	 */
	UFUNCTION(BlueprintCallable, Category = "Bow")
	virtual void SetBowArmed(bool bArmed);

	/**
	 * 获取当前是否处于弓箭模式
	 * @return true=处于弓箭模式，false=未处于弓箭模�?
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Bow")
	virtual bool GetBowArmed() const;

	UFUNCTION(BlueprintCallable, Category = "Controller")
	virtual void PlaySimpleForceFeedback(EControllerHand Hand);
	UFUNCTION(BlueprintCallable, Category = "Climb")
	virtual void EnterClimbState();
	UFUNCTION(BlueprintCallable, Category = "Climb")
	virtual void ExitClimbState();
	UFUNCTION(BlueprintCallable, Category = "Climb")
	void RegisterClimbGrip(UPlayerGrabHand* Hand, AActor* ClimbActor);
	UFUNCTION(BlueprintCallable, Category = "Climb")
	void UnregisterClimbGrip(UPlayerGrabHand* Hand, AActor* ClimbActor);
	UFUNCTION(BlueprintCallable, Category = "Climb")
	bool HasAnyValidClimbGrip();
	UFUNCTION(BlueprintCallable, Category = "Climb")
	void TryExitClimbStateIfNoValidGrip();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Climb")
	bool IsInClimbState() const { return bInClimbState; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Climb")
	float GetCapsuleBottomZ() const;
	
	//===================== GrassHide ====================
	UFUNCTION(BlueprintCallable, Category = "GrassHide")
	void SetCameraInGrass(bool bInGrass);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bow")
	APlayerController* PlayerController;

	// ==================== 弓相�?====================
	
	/** 是否已获得弓（游戏流程中获得，永久持有） */
	UPROPERTY(BlueprintReadOnly, Category = "Bow")
	bool bHasBow = false;

	/** 是否处于弓箭模式 */
	UPROPERTY(BlueprintReadOnly, Category = "Bow")
	bool bIsBowArmed = false;


	/** 当前持有的弓（弓箭模式下不为空） */
	UPROPERTY(BlueprintReadOnly, Category = "Bow")
	ABow* CurrentBow = nullptr;
	
	UPROPERTY(Transient)
	bool bCameraInGrass = false;
	UPROPERTY(Transient)
	bool bInClimbState = false;
	UPROPERTY(Transient)
	FName CachedCapsuleCollisionProfileBeforeClimb = NAME_None;
	UPROPERTY(Transient)
	uint8 CachedMovementModeBeforeClimb = 0;
	UPROPERTY(Transient)
	uint8 CachedCustomMovementModeBeforeClimb = 0;
	UPROPERTY(Transient)
	float CachedGravityScaleBeforeClimb = 1.0f;

	/** 生成�?Actor */
	virtual ABow* SpawnBow();

	/** 销毁弓 Actor */
	virtual void DestroyBow();
};

