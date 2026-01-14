// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Skill/SkillTypes.h"
#include "SkillStrategyBase.generated.h"

class ABasePlayer;

/**
 * 技能策略基类：每个具体技能实现一个策略 Actor。
 *
 * 框架约束：策略只关心“如何执行技能”，不处理技能学习状态，也不管理绘制会话生命周期。
 */
UCLASS(Abstract, Blueprintable)
class VRTEST_API ASkillStrategyBase : public AActor
{
	GENERATED_BODY()

public:
	ASkillStrategyBase()
	{
		PrimaryActorTick.bCanEverTick = false;
	}

	/** 执行技能。返回值表示是否成功触发（可以用于日志/调试/UI）。 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Skill")
	bool Execute(ABasePlayer* Player, const FSkillContext& Context);
	virtual bool Execute_Implementation(ABasePlayer* Player, const FSkillContext& Context) { return false; }
};
