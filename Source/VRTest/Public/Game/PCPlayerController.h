// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PCPlayerController.generated.h"

class UUserWidget;

/**
 * PC 模式玩家控制器
 *
 * 负责暂停菜单管理和全局玩家输入锁。
 */
UCLASS()
class VRTEST_API APCPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APCPlayerController();

	/**
	 * 设置全局玩家输入锁。
	 * false = 玩家不可操作（主菜单、镜头过渡等），所有输入被屏蔽。
	 * true = 玩家可正常操作。
	 */
	UFUNCTION(BlueprintCallable, Category = "PlayerInput")
	void SetPlayerInputEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "PlayerInput")
	bool IsPlayerInputEnabled() const { return bIsPlayerInputEnabled; }

	/** 玩家是否可以正常行动（用于每个 BP 输入事件开头的守卫判断） */
	UFUNCTION(BlueprintPure, Category = "PauseMenu")
	bool CanPlayerAct() const { return !bIsPaused && bIsPlayerInputEnabled; }

	/**
	 * 暂停游戏：显示暂停菜单、暂停游戏逻辑、显示鼠标。
	 * 由蓝图输入事件调用（ESC 键）。
	 */
	UFUNCTION(BlueprintCallable, Category = "PauseMenu")
	void PauseGame();

	/**
	 * 恢复游戏：隐藏暂停菜单、恢复游戏逻辑、隐藏鼠标。
	 * 由蓝图输入事件或暂停菜单按钮调用。
	 */
	UFUNCTION(BlueprintCallable, Category = "PauseMenu")
	void ResumeGame();

	/**
	 * 切换暂停/恢复。ESC 输入事件统一调用此函数即可。
	 */
	UFUNCTION(BlueprintCallable, Category = "PauseMenu")
	void TogglePause();

	/** 当前是否处于暂停状态 */
	UFUNCTION(BlueprintPure, Category = "PauseMenu")
	bool IsGamePaused() const { return bIsPaused; }

protected:
	virtual void BeginPlay() override;
	virtual bool InputKey(const FInputKeyParams& Params) override;

private:
	/** 暂停菜单实例 */
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> PauseMenuInstance;

	bool bIsPaused = false;

	/** 全局玩家输入锁：false 时 TogglePause/PauseGame 不做任何事 */
	bool bIsPlayerInputEnabled = false;
};
