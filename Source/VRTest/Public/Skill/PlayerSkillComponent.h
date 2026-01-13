// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Skill/SkillTypes.h"
#include "PlayerSkillComponent.generated.h"

class ABasePlayer;
class AStarDrawManager;
class USkillStrategyBase;

/**
 * 玩家技能组件：
 * - 管理玩家技能学习状态
 * - 管理星图绘制状态与会话（Spawn/持有 StarDrawManager）
 * - 作为技能触发入口（路由到策略）
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VRTEST_API UPlayerSkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerSkillComponent();

	// ==================== StarDraw 入口 ====================

	/**
	 * 开始绘制（统一入口）。
	 * @param InputSource PC=Camera，VR=Hand。
	 * @param bIsRight true=右手绘制，false=左手绘制。
	 */
	UFUNCTION(BlueprintCallable, Category = "Skill|StarDraw")
	bool StartStarDraw(USceneComponent* InputSource, bool bIsRight);

	/** 结束绘制（统一入口）。返回识别到的技能类型（None 表示无效）。 */
	UFUNCTION(BlueprintCallable, Category = "Skill|StarDraw")
	ESkillType FinishStarDraw();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Skill|StarDraw")
	bool IsDrawing() const { return bIsDrawing; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Skill|StarDraw")
	bool IsRightHandDrawing() const { return bIsRightHandDrawing; }

	// ==================== 学习状态 ====================

	UFUNCTION(BlueprintCallable, Category = "Skill|Learn")
	void LearnSkill(ESkillType SkillType);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Skill|Learn")
	bool HasLearnedSkill(ESkillType SkillType) const;

	// ==================== 技能触发 ====================

	/** 直接触发技能（绕过绘制），通常用于调试/快捷键。 */
	UFUNCTION(BlueprintCallable, Category = "Skill|Cast")
	bool TryCastSkill(ESkillType SkillType, const FSkillContext& Context);

protected:
	virtual void BeginPlay() override;

	// 不提供 GetOwnerPlayer()，统一在 BeginPlay 缓存并直接使用 CachedOwnerPlayer。

	bool CanStartDraw_PC_BowGate(const ABasePlayer* Player) const;

	AStarDrawManager* SpawnStarDrawManager();
	void DestroyStarDrawManager();

	USkillStrategyBase* GetStrategyForSkill(ESkillType SkillType) const;

protected:
	// ==================== 学习状态 ====================

	UPROPERTY(BlueprintReadOnly, Category = "Skill|Learn")
	TSet<ESkillType> LearnedSkills;

	// ==================== 策略映射 ====================

	/**
	 * SkillType -> StrategyClass 映射。
	 * 由 GameSettings->SkillAsset 在 BeginPlay 中初始化（DataAsset 驱动）。
	 */
	UPROPERTY(Transient)
	TMap<ESkillType, TSubclassOf<USkillStrategyBase>> StrategyClassMap;

	UPROPERTY(Transient)
	mutable TMap<ESkillType, TObjectPtr<USkillStrategyBase>> StrategyInstanceCache;

	/** 缓存的 Owner（BasePlayer）。BeginPlay 中初始化。 */
	UPROPERTY(Transient)
	TObjectPtr<ABasePlayer> CachedOwnerPlayer;

	// ==================== 状态 ====================

	UPROPERTY(BlueprintReadOnly, Category = "Skill|State")
	bool bIsDrawing = false;

	UPROPERTY(BlueprintReadOnly, Category = "Skill|State")
	bool bIsRightHandDrawing = true;

	/** 缓存本次绘制会话的上下文（输入源/手别等）。 */
	UPROPERTY(Transient)
	FSkillContext CachedDrawContext;

	UPROPERTY(BlueprintReadOnly, Category = "Skill|State")
	TObjectPtr<AStarDrawManager> StarDrawManager;
	
};
