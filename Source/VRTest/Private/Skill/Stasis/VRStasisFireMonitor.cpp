// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/Stasis/VRStasisFireMonitor.h"
#include "Skill/Stasis/StasisPoint.h"
#include "Grabber/PlayerGrabHand.h"

AVRStasisFireMonitor::AVRStasisFireMonitor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
}

void AVRStasisFireMonitor::BeginPlay()
{
	Super::BeginPlay();
}

void AVRStasisFireMonitor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!MonitoredHand || !StasisPoint)
	{
		// 手部或定身球无效，销毁自身
		Destroy();
		return;
	}

	// 检查手部是否仍然持有定身球
	if (MonitoredHand->HeldActor != StasisPoint)
	{
		// 定身球已被释放（可能被其他逻辑处理），销毁监视器
		Destroy();
		return;
	}

	// 更新手部速度
	UpdateHandVelocity(DeltaTime);

	// 检测速度是否超过阈值
	if (!bSpeedOverThreshold && CurrentSpeed > SpeedThreshold)
	{
		bSpeedOverThreshold = true;
	}

	// 如果速度已超过阈值，且现在开始下降，触发发射
	if (bSpeedOverThreshold && CurrentSpeed < LastSpeed)
	{
		FireStasisPoint();
	}
}

void AVRStasisFireMonitor::Initialize(UPlayerGrabHand* InHand, AStasisPoint* InStasisPoint)
{
	MonitoredHand = InHand;
	StasisPoint = InStasisPoint;

	if (MonitoredHand)
	{
		LastHandLocation = MonitoredHand->GetComponentLocation();
		CurrentVelocity = FVector::ZeroVector;
		LastVelocity = FVector::ZeroVector;
		CurrentSpeed = 0.0f;
		LastSpeed = 0.0f;
		bSpeedOverThreshold = false;
	}
}

void AVRStasisFireMonitor::UpdateHandVelocity(float DeltaTime)
{
	if (!MonitoredHand || DeltaTime <= 0.0f)
	{
		return;
	}

	FVector CurrentLocation = MonitoredHand->GetComponentLocation();
	
	// 保存上一帧的速度
	LastVelocity = CurrentVelocity;
	LastSpeed = CurrentSpeed;

	// 计算当前速度
	CurrentVelocity = (CurrentLocation - LastHandLocation) / DeltaTime;
	CurrentSpeed = CurrentVelocity.Size();

	// 更新位置
	LastHandLocation = CurrentLocation;
}

void AVRStasisFireMonitor::FireStasisPoint()
{
	if (!MonitoredHand || !StasisPoint)
	{
		return;
	}

	// 1) 计算发射上下文（VR：基于手部 LastVelocity 方向）
	const FVector HandLocation = MonitoredHand->GetComponentLocation();
	const FVector VelocityDirection = LastVelocity.GetSafeNormal();

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(MonitoredHand->GetOwner()); // 忽略玩家自身
	IgnoreActors.Add(StasisPoint); // 忽略定身球自身

	// 2) 计算发射速度
	const FVector InitVelocity = LastVelocity * FireSpeedFactor;

	// 3) 释放定身球
	MonitoredHand->ReleaseObject();

	// 4) 发射：由定身球内部自行找目标
	StasisPoint->Fire(
		this,
		HandLocation,
		VelocityDirection,
		InitVelocity,
		DetectionRadius,
		DetectionAngle,
		IgnoreActors
	);

	// 5) 解锁手部
	MonitoredHand->SetGrabLock(false);

	// 6) 销毁监视器
	Destroy();
}
