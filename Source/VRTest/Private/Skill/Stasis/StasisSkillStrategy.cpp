// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/Stasis/StasisSkillStrategy.h"
#include "Skill/Stasis/StasisPoint.h"
#include "Skill/Stasis/VRStasisFireMonitor.h"
#include "Game/BasePlayer.h"
#include "Game/ShujiGameMode.h"
#include "Grabber/PlayerGrabHand.h"
#include "Skill/SkillTypes.h"

AStasisSkillStrategy::AStasisSkillStrategy()
{
	// 默认使用 AStasisPoint 类
	StasisPointClass = AStasisPoint::StaticClass();
	
	// 默认使用 AVRStasisFireMonitor 类
	VRFireMonitorClass = AVRStasisFireMonitor::StaticClass();
}

bool AStasisSkillStrategy::Execute_Implementation(ABasePlayer* Player, const FSkillContext& Context)
{
	if (!Player || !Context.InputSource)
	{
		UE_LOG(LogTemp, Warning, TEXT("AStasisSkillStrategy::Execute: Invalid Player or InputSource"));
		return false;
	}

	// 根据 Context 确定使用哪只手
	UPlayerGrabHand* TargetHand = Context.bIsRightHand ? Player->RightHand : Player->LeftHand;

	if (!TargetHand)
	{
		UE_LOG(LogTemp, Warning, TEXT("AStasisSkillStrategy::Execute: Target hand is null"));
		return false;
	}

	// 检查手部是否已经持有物体
	if (TargetHand->bIsHolding)
	{
		UE_LOG(LogTemp, Warning, TEXT("AStasisSkillStrategy::Execute: Hand is already holding an object"));
		return false;
	}

	// 在手部位置 Spawn StasisPoint
	FVector SpawnLocation = Context.InputSource->GetComponentLocation();
	FRotator SpawnRotation = Context.InputSource->GetComponentRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Player;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AStasisPoint* StasisPoint = GetWorld()->SpawnActor<AStasisPoint>(
		StasisPointClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (!StasisPoint)
	{
		UE_LOG(LogTemp, Warning, TEXT("AStasisSkillStrategy::Execute: Failed to spawn StasisPoint"));
		return false;
	}

	// 锁定手部，防止玩家意外释放（通过 TryRelease）
	TargetHand->SetGrabLock(true);
	
	// 调用手部 GrabObject 直接抓取定身球
	TargetHand->GrabObject(StasisPoint);

	// 根据游戏模式判断是否需要生成 VR 监视器
	UWorld* World = GetWorld();
	if (World)
	{
		AShujiGameMode* GameMode = Cast<AShujiGameMode>(World->GetAuthGameMode());
		if (GameMode && GameMode->GetIsVRMode())
		{
			// VR 模式：生成 VRStasisFireMonitor 监视手部速度
			if (VRFireMonitorClass)
			{
				FActorSpawnParameters MonitorSpawnParams;
				MonitorSpawnParams.Owner = Player;
				MonitorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				AVRStasisFireMonitor* FireMonitor = World->SpawnActor<AVRStasisFireMonitor>(
					VRFireMonitorClass,
					SpawnLocation,
					FRotator::ZeroRotator,
					MonitorSpawnParams
				);

				if (FireMonitor)
				{
					FireMonitor->Initialize(TargetHand, StasisPoint);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("AStasisSkillStrategy::Execute: Failed to spawn VRStasisFireMonitor"));
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("AStasisSkillStrategy::Execute: VRFireMonitorClass is not set"));
			}
		}
		// PC 模式：不生成监视器，等待玩家主动投掷（在 BasePCPlayer::TryThrow 中处理）
	}

	// 技能触发成功
	return true;
}

