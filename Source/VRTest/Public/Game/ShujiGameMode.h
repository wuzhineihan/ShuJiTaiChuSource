// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShujiGameMode.generated.h"

class UUserWidget;
class ACameraActor;

/**
 * 主游戏模式
 *
 * 负责 PC/VR Pawn 选择和主菜单流程控制。
 */
UCLASS()
class VRTEST_API AShujiGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AShujiGameMode();

	// ==================== PC/VR Pawn 选择 ====================

	UPROPERTY(EditDefaultsOnly, Category = "Classes")
	TSubclassOf<APawn> VRPawnClass;

	UPROPERTY(EditDefaultsOnly, Category = "Classes")
	TSubclassOf<APawn> PCPawnClass;

	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	UFUNCTION(BlueprintPure, Category = "GameMode")
	bool GetIsVRMode();

	// ==================== 主菜单系统 ====================

	/** 菜单摄像机 Actor 的 Tag（在地图中给 CameraActor 设置此 Tag） */
	UPROPERTY(EditDefaultsOnly, Category = "MainMenu")
	FName MenuCameraTag = TEXT("MenuCamera");

	/** 从菜单镜头 Blend 到玩家视角的时长（秒） */
	UPROPERTY(EditDefaultsOnly, Category = "MainMenu")
	float CameraBlendDuration = 2.0f;

	/** Blend 曲线指数：越大起点终点越平缓、中间越快（推荐 1.5~3.0） */
	UPROPERTY(EditDefaultsOnly, Category = "MainMenu", meta = (ClampMin = "0.1", UIMin = "0.5", UIMax = "5.0"))
	float CameraBlendExponent = 2.0f;

	/** 主菜单控件蓝图类（拖入你的 MenuWidget 蓝图） */
	UPROPERTY(EditDefaultsOnly, Category = "MainMenu")
	TSubclassOf<UUserWidget> MainMenuWidgetClass;

	/** 暂停菜单控件蓝图类（在 GameMode 蓝图中设置） */
	UPROPERTY(EditDefaultsOnly, Category = "PauseMenu")
	TSubclassOf<UUserWidget> PauseMenuWidgetClass;

	/** 读档黑屏控件蓝图类 */
	UPROPERTY(EditDefaultsOnly, Category = "SaveGame")
	TSubclassOf<UUserWidget> LoadingScreenWidgetClass;

	/** 读档黑屏最小显示时间（秒） */
	UPROPERTY(EditDefaultsOnly, Category = "SaveGame", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "5.0"))
	float LoadingScreenMinDuration = 1.0f;

	// ==================== PC HUD ====================

	/** PC 模式的 HUD 控件蓝图（游戏中显示，菜单/暂停时隐藏） */
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UUserWidget> PCHUDWidgetClass;

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ShowPCHUD();

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void HidePCHUD();

	UFUNCTION(BlueprintPure, Category = "HUD")
	bool IsPCHUDVisible() const { return PCHUDInstance != nullptr; }

	// ==================== 返回主菜单 ====================

	/** 临时加载关卡（直接拖入你的 LoadingLevel.umap） */
	UPROPERTY(EditDefaultsOnly, Category = "MainMenu")
	TSoftObjectPtr<UWorld> LoadingLevel;

	/**
	 * 开始游戏：由 UI 蓝图在菜单动画结束后调用。
	 * - 移除菜单 Widget
	 * - 摄像机 Blend 到玩家 Pawn（位置 + 旋转）
	 * - Blend 完成后恢复输入、隐藏鼠标
	 */
	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void StartGame();

	/** Blend 过渡+输入恢复完成后触发，用于在蓝图中执行玩家就绪逻辑 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMenuTransitionComplete);

	UPROPERTY(BlueprintAssignable, Category = "MainMenu")
	FOnMenuTransitionComplete OnMenuTransitionComplete;

	// ==================== 存档系统 ====================

	/** 存档槽位名称 */
	static const FString SaveSlotName;

	/** 保存游戏 */
	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void SaveGame();

	/** 读取存档：黑屏 → 恢复数据 → 直接控制玩家（无镜头过渡） */
	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void LoadGame();

	/** 检查是否存在存档 */
	UFUNCTION(BlueprintPure, Category = "SaveGame")
	bool HasSaveData() const;

	// ==================== 主菜单导航 ====================

	/** 返回主菜单：跳转到临时加载关卡，那里负责异步加载主地图 */
	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void ReturnToMainMenu();

protected:
	virtual void BeginPlay() override;

	/** 按 Tag 查找地图中的菜单 CameraActor */
	ACameraActor* FindMenuCamera() const;

	/** 菜单 Widget 运行时实例（蓝图可读，用于绑定委托） */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "MainMenu")
	TObjectPtr<UUserWidget> MainMenuWidgetInstance;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> LoadingScreenInstance;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> PCHUDInstance;

private:

	/** 防止 StartGame 重复调用 */
	bool bIsStartingGame = false;

	/** 当前是否为 VR 模式 */
	bool bIsVRMode = false;
};
