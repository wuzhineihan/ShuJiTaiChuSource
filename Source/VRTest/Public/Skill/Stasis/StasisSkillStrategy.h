// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Skill/SkillStrategyBase.h"
#include "StasisSkillStrategy.generated.h"

class AStasisPoint;
class AVRStasisFireMonitor;

/**
 * 定身术技能策略
 * 
 * 执行逻辑：
 * 1. 在玩家绘制手位置 Spawn StasisPoint
 * 2. 调用手部 GrabObject 直接抓取定身球
 * 3. 锁定该手部，防止玩家意外释放
 * 4. 根据游戏模式（通过 GameMode 判断）：
 *    - PC：等待玩家投掷操作（在 TryThrow 中处理）
 *    - VR：生成 VRStasisFireMonitor 监视手部速度，自动触发发射
 */
UCLASS()
class VRTEST_API AStasisSkillStrategy : public ASkillStrategyBase
{
	GENERATED_BODY()

public:
	AStasisSkillStrategy();

	virtual bool Execute_Implementation(ABasePlayer* Player, const FSkillContext& Context) override;

protected:
	/** StasisPoint Actor 类（可在蓝图中配置） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stasis")
	TSubclassOf<AStasisPoint> StasisPointClass;

	/** VR 模式下的发射监视器类（可在蓝图中配置） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stasis")
	TSubclassOf<AVRStasisFireMonitor> VRFireMonitorClass;
};

