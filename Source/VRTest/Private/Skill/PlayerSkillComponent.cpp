// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/PlayerSkillComponent.h"

#include "Skill/StarDrawManager.h"
#include "Skill/SkillStrategyBase.h"
#include "Game/BasePlayer.h"
#include "Game/BasePCPlayer.h"
#include "Grabber/PlayerGrabHand.h"
#include "Game/GameSettings.h"
#include "Skill/SkillAsset.h"

#include "Engine/World.h"

UPlayerSkillComponent::UPlayerSkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerSkillComponent::BeginPlay()
{
	Super::BeginPlay();

	CachedOwnerPlayer = Cast<ABasePlayer>(GetOwner());

	// DataAsset 驱动：从 GameSettings 读取 SkillAsset 并初始化策略映射
	if (const UGameSettings* Settings = UGameSettings::Get())
	{
		if (USkillAsset* Asset = Settings->GetSkillAsset())
		{
			StrategyClassMap = Asset->StrategyClassMap;
			StrategyInstanceCache.Reset();
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("SkillComponent: GameSettings not found"));
	}
}

bool UPlayerSkillComponent::StartStarDraw(USceneComponent* InputSource, bool bIsRight)
{
	if (bIsDrawing)
	{
		return false;
	}

	ABasePlayer* Player = CachedOwnerPlayer;
	if (!Player || !InputSource)
	{
		return false;
	}

	// PC 弓箭模式 Gate：弓箭模式不允许启动绘制
	if (!CanStartDraw_PC_BowGate(Player))
	{
		return false;
	}

	// 绘制互斥 Gate：指定的绘制手若正在抓取，则禁止开始绘制
	if (bIsRight)
	{
		if (Player->RightHand && Player->RightHand->bIsHolding)
		{
			return false;
		}
	}
	else
	{
		if (Player->LeftHand && Player->LeftHand->bIsHolding)
		{
			return false;
		}
	}

	bIsRightHandDrawing = bIsRight;
	bIsDrawing = true;

	// 缓存本次输入源，供 Finish/Strategy 使用
	FSkillContext Context;
	Context.InputSource = InputSource;
	Context.bIsRightHand = bIsRightHandDrawing;
	CachedDrawContext = Context;

	StarDrawManager = SpawnStarDrawManager();
	if (!StarDrawManager)
	{
		bIsDrawing = false;
		return false;
	}

	StarDrawManager->StartDraw(InputSource);
	return true;
}

ESkillType UPlayerSkillComponent::FinishStarDraw()
{
	if (!bIsDrawing)
	{
		return ESkillType::None;
	}

	bIsDrawing = false;

	ESkillType Result = ESkillType::None;
	if (StarDrawManager)
	{
		Result = StarDrawManager->FinishDraw();
	}

	DestroyStarDrawManager();

	// 只要识别结果有效，就尝试触发
	if (Result != ESkillType::None)
	{
		FSkillContext Context;
		Context.InputSource = CachedDrawContext.InputSource;
		Context.bIsRightHand = bIsRightHandDrawing;
		TryCastSkill(Result, Context);
	}

	return Result;
}

void UPlayerSkillComponent::LearnSkill(ESkillType SkillType)
{
	if (SkillType != ESkillType::None)
	{
		LearnedSkills.Add(SkillType);
	}
}

bool UPlayerSkillComponent::HasLearnedSkill(ESkillType SkillType) const
{
	return LearnedSkills.Contains(SkillType);
}

bool UPlayerSkillComponent::TryCastSkill(ESkillType SkillType, const FSkillContext& Context)
{
	ABasePlayer* Player = CachedOwnerPlayer;
	if (!Player)
	{
		return false;
	}

	if (SkillType == ESkillType::None)
	{
		return false;
	}

	if (!HasLearnedSkill(SkillType))
	{
		return false;
	}

	USkillStrategyBase* Strategy = GetStrategyForSkill(SkillType);
	if (!Strategy)
	{
		return false;
	}

	return Strategy->Execute(Player, Context);
}

bool UPlayerSkillComponent::CanStartDraw_PC_BowGate(const ABasePlayer* Player) const
{
	// 只有 PC 有“弓箭模式禁止绘制”的规则
	const ABasePCPlayer* PCPlayer = Cast<ABasePCPlayer>(Player);
	if (!PCPlayer)
	{
		return true;
	}

	return !PCPlayer->GetBowArmed();
}

AStarDrawManager* UPlayerSkillComponent::SpawnStarDrawManager()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.Owner = GetOwner();
	return World->SpawnActor<AStarDrawManager>(AStarDrawManager::StaticClass(), Params);
}

void UPlayerSkillComponent::DestroyStarDrawManager()
{
	if (StarDrawManager)
	{
		StarDrawManager->Destroy();
		StarDrawManager = nullptr;
	}
}

USkillStrategyBase* UPlayerSkillComponent::GetStrategyForSkill(ESkillType SkillType) const
{
	if (const TObjectPtr<USkillStrategyBase>* Cached = StrategyInstanceCache.Find(SkillType))
	{
		return Cached->Get();
	}

	const TSubclassOf<USkillStrategyBase>* StrategyClass = StrategyClassMap.Find(SkillType);
	if (!StrategyClass || !(*StrategyClass))
	{
		return nullptr;
	}

	USkillStrategyBase* StrategyObj = NewObject<USkillStrategyBase>(const_cast<UPlayerSkillComponent*>(this), *StrategyClass);
	StrategyInstanceCache.Add(SkillType, StrategyObj);
	return StrategyObj;
}
