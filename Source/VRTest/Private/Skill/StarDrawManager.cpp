// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/StarDrawManager.h"

#include "Skill/StarDrawFingerPoint.h"
#include "Skill/StarDrawMainStar.h"
#include "Skill/StarDrawOtherStar.h"
#include "Skill/SkillAsset.h"
#include "Skill/SkillTypes.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"
#include "Game/GameSettings.h"

void AStarDrawManager::BeginPlay()
{
	Super::BeginPlay();

	const UGameSettings* Settings = UGameSettings::Get();
	CachedSkillAsset = Settings ? Settings->GetSkillAsset() : nullptr;
}

AStarDrawManager::AStarDrawManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AStarDrawManager::StartDraw(USceneComponent* InInputSource)
{
	InputSource = InInputSource;
	bIsDrawing = (InputSource != nullptr);
	CachedResult = ESkillType::None;

	DirectionChoose.Reset();
	MainStars.Reset();
	OtherStars.Reset();
	FingerPoint = nullptr;

	if (!bIsDrawing)
	{
		return;
	}

	FirstTouch();
}

ESkillType AStarDrawManager::FinishDraw()
{
	if (!bIsDrawing)
	{
		return ESkillType::None;
	}

	bIsDrawing = false;
	CachedResult = ResolveSkillFromDraw();

	if (FingerPoint)
	{
		FingerPoint->Destroy();
		FingerPoint = nullptr;
	}

	ClearOtherStars();
	ClearMainStars();

	return CachedResult;
}


void AStarDrawManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bIsDrawing || !FingerPoint)
	{
		return;
	}

	UpdateFingerPointLocation(DeltaSeconds);

	if (bDebugDraw)
	{
		DrawDebugPoint(GetWorld(), FingerPointLocation, 6.f, FColor::Green, false, 0.f);
	}
}


void AStarDrawManager::NotifyFingerTouchActor(AActor* TouchedActor)
{
	if (!bIsDrawing || !TouchedActor)
	{
		return;
	}

	// 统一：lower_snake_case
	if (!TouchedActor->ActorHasTag(SkillTags::OtherStars))
	{
		return;
	}

	AStarDrawOtherStar* OtherStar = Cast<AStarDrawOtherStar>(TouchedActor);
	if (!OtherStar)
	{
		return;
	}

	TouchOtherStar(OtherStar);
}


void AStarDrawManager::FirstTouch()
{
	PlayerPivotLocation = GetPlayerPivotLocation();

	const FVector ControllerLocation = GetInputSourceLocation();
	const FVector Forward = GetInputSourceForwardVector();
	Radius = CalculateRadiusOnXY(PlayerPivotLocation, ControllerLocation, Forward, Distance);

	// 蓝图：FingerPointLocation = ControllerLocation + Forward * Distance
	FingerPointLocation = ControllerLocation + Forward * Distance;

	SpawnFingerPoint();
	SpawnMainStarAt(FingerPointLocation);
	SpawnOtherStarsAround(FingerPointLocation);
}


void AStarDrawManager::TouchOtherStar(AStarDrawOtherStar* OtherStar)
{
	if (!OtherStar)
	{
		return;
	}

	const EStarDrawDirection CurrentDir = OtherStar->GetDirection();

	// 蓝图：如果不是上一步的反方向才记录（避免来回抖动）
	if (DirectionChoose.Num() > 0)
	{
		const EStarDrawDirection Last = DirectionChoose.Last();
		if (StarDrawDirectionGetOpposite(CurrentDir) == Last)
		{
			return;
		}
	}

	DirectionChoose.Add(CurrentDir);

	const FVector HitLocation = OtherStar->GetActorLocation();
	SpawnMainStarAt(HitLocation);
	SpawnOtherStarsAround(HitLocation);
}


void AStarDrawManager::SpawnFingerPoint()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 优先从 SkillAsset 读取可视化蓝图类
	if (CachedSkillAsset && CachedSkillAsset->FingerPointClass)
	{
		FingerPointClass = CachedSkillAsset->FingerPointClass;
	}

	if (!FingerPointClass)
	{
		FingerPointClass = AStarDrawFingerPoint::StaticClass();
	}

	FActorSpawnParameters Params;
	Params.Owner = this;
	FingerPoint = World->SpawnActor<AStarDrawFingerPoint>(FingerPointClass, FingerPointLocation, FRotator::ZeroRotator, Params);
	if (FingerPoint)
	{
		FingerPoint->SetDrawManager(this);
		FingerPoint->SetActorLocation(FingerPointLocation);
	}
}

void AStarDrawManager::SpawnMainStarAt(const FVector& Location)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (CachedSkillAsset && CachedSkillAsset->MainStarClass)
	{
		MainStarClass = CachedSkillAsset->MainStarClass;
	}

	if (!MainStarClass)
	{
		MainStarClass = AStarDrawMainStar::StaticClass();
	}

	FActorSpawnParameters Params;
	Params.Owner = this;
	AStarDrawMainStar* Star = World->SpawnActor<AStarDrawMainStar>(MainStarClass, Location, FRotator::ZeroRotator, Params);
	if (Star)
	{
		MainStars.Add(Star);
	}
}

void AStarDrawManager::SpawnOtherStarsAround(const FVector& CenterLocation)
{
	ClearOtherStars();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (CachedSkillAsset && CachedSkillAsset->OtherStarClass)
	{
		OtherStarClass = CachedSkillAsset->OtherStarClass;
	}

	if (!OtherStarClass)
	{
		OtherStarClass = AStarDrawOtherStar::StaticClass();
	}

	const TMap<EStarDrawDirection, FVector> PointsByDir = CalculateAdjacentPointsOnCylinder(PlayerPivotLocation, CenterLocation, LineLength);
	if (PointsByDir.Num() != 8)
	{
		return;
	}

	for (const TPair<EStarDrawDirection, FVector>& Pair : PointsByDir)
	{
		const EStarDrawDirection Dir = Pair.Key;
		if (DirectionChoose.Num()>0 && StarDrawDirectionGetOpposite(Dir) == DirectionChoose.Last())
		{
			// 不生成上一步的反方向
			continue;
		}
		const FVector& SpawnLoc = Pair.Value;

		FActorSpawnParameters Params;
		Params.Owner = this;
		AStarDrawOtherStar* Other = World->SpawnActor<AStarDrawOtherStar>(OtherStarClass, SpawnLoc, FRotator::ZeroRotator, Params);
		if (Other)
		{
			Other->SetDirection(Dir);
			OtherStars.Add(Other);
		}
	}
}


void AStarDrawManager::ClearOtherStars()
{
	for (AStarDrawOtherStar* Star : OtherStars)
	{
		if (Star)
		{
			Star->Destroy();
		}
	}
	OtherStars.Reset();
}


void AStarDrawManager::ClearMainStars()
{
	for (AStarDrawMainStar* Star : MainStars)
	{
		if (Star)
		{
			Star->Destroy();
		}
	}
	MainStars.Reset();
}


FVector AStarDrawManager::GetPlayerPivotLocation() const
{
	// 蓝图里是 TriggerCameraLocation = PlayerCameraManager.GetCameraLocation()
	// C++ 里优先从 InputSource 所属的 PlayerController/CameraManager 获取（PC/VR 均可）
	if (APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
	{
		return PC->PlayerCameraManager ? PC->PlayerCameraManager->GetCameraLocation() : FVector::ZeroVector;
	}
	return FVector::ZeroVector;
}


FVector AStarDrawManager::GetInputSourceLocation() const
{
	return InputSource ? InputSource->GetComponentLocation() : FVector::ZeroVector;
}

FVector AStarDrawManager::GetInputSourceForwardVector() const
{
	if (!InputSource)
	{
		return FVector::ForwardVector;
	}

	// 旧蓝图：Forward = MotionController.GetUpVector() * -1
	// 为了兼顾 PC(Camera) 与 VR(Hand)，这里做一个折中：
	// - 如果是 Camera：ForwardVector
	// - 如果是手：也可以直接用 ForwardVector（或由蓝图在输入源上调整朝向）。
	// 当前默认使用 ForwardVector。
	return InputSource->GetForwardVector();
}

void AStarDrawManager::UpdateFingerPointLocation(float DeltaSeconds)
{
	const FVector ControllerLocation = GetInputSourceLocation();
	const FVector Forward = GetInputSourceForwardVector();
	FingerPointLocation = CalculateProjectionPointOnCylinder(PlayerPivotLocation, ControllerLocation, Forward, Radius);
	FingerPoint->SetActorLocation(FingerPointLocation);
}

TMap<EStarDrawDirection, FVector> AStarDrawManager::CalculateAdjacentPointsOnCylinder(const FVector& PlayerPivot, const FVector& CenterLocation, float InLineLength) const
{
	// 对齐 Z（在蓝图库里 PlayerLocation.Z=StarLocation.Z）
	FVector PlayerFlat = PlayerPivot;
	PlayerFlat.Z = CenterLocation.Z;

	const FVector OriginalVector = CenterLocation - PlayerFlat;
	const float RadiusLocal = FVector(OriginalVector.X, OriginalVector.Y, 0.f).Size();
	if (RadiusLocal <= KINDA_SMALL_NUMBER)
	{
		return {};
	}

	// Zita = acos((2R^2 - L^2) / (2R^2))
	const float CosValue = (2.f * RadiusLocal * RadiusLocal - InLineLength * InLineLength) / (2.f * RadiusLocal * RadiusLocal);
	const float ClampedCos = FMath::Clamp(CosValue, -1.f, 1.f);
	const float ZitaDeg = FMath::RadiansToDegrees(FMath::Acos(ClampedCos));

	FVector OriginalFlat = OriginalVector;
	OriginalFlat.Z = 0.f;

	const FVector LeftVector = OriginalFlat.RotateAngleAxis(ZitaDeg, FVector::UpVector);
	const FVector RightVector = OriginalFlat.RotateAngleAxis(-ZitaDeg, FVector::UpVector);

	const FVector LeftPoint = PlayerFlat + LeftVector;
	const FVector RightPoint = PlayerFlat + RightVector;

	const FVector UpVector = FVector(0, 0, InLineLength);
	const FVector DownVector = FVector(0, 0, -InLineLength);

	const FVector UpPoint = CenterLocation + UpVector;
	const FVector DownPoint = CenterLocation + DownVector;
	const FVector LeftUpPoint = LeftPoint + UpVector;
	const FVector RightUpPoint = RightPoint + UpVector;
	const FVector LeftDownPoint = LeftPoint + DownVector;
	const FVector RightDownPoint = RightPoint + DownVector;

	TMap<EStarDrawDirection, FVector> Result;
	Result.Reserve(8);
	Result.Add(EStarDrawDirection::Left, LeftPoint);
	Result.Add(EStarDrawDirection::Right, RightPoint);
	Result.Add(EStarDrawDirection::Up, UpPoint);
	Result.Add(EStarDrawDirection::Down, DownPoint);
	Result.Add(EStarDrawDirection::LeftUp, LeftUpPoint);
	Result.Add(EStarDrawDirection::RightUp, RightUpPoint);
	Result.Add(EStarDrawDirection::LeftDown, LeftDownPoint);
	Result.Add(EStarDrawDirection::RightDown, RightDownPoint);
	return Result;
}

float AStarDrawManager::CalculateRadiusOnXY(const FVector& PlayerPivot, const FVector& ControllerLocation, const FVector& Forward, float DistanceValue) const
{
	FVector ForwardFlat = Forward;
	ForwardFlat.Z = 0.f;
	ForwardFlat = ForwardFlat.GetSafeNormal();
	const FVector TargetLocation = ControllerLocation + ForwardFlat * DistanceValue;
	return FVector(TargetLocation.X - PlayerPivot.X, TargetLocation.Y - PlayerPivot.Y, 0.f).Size();
}

FVector AStarDrawManager::CalculateProjectionPointOnCylinder(const FVector& PlayerPivot, const FVector& ControllerLocation, const FVector& Forward, float RadiusValue) const
{
	// 将问题约化到 XY：光线与圆（以 PlayerPivot 为圆心，半径 RadiusValue）的交点
	const FVector2D PlayerXY(PlayerPivot.X, PlayerPivot.Y);
	const FVector2D ControllerXY(ControllerLocation.X, ControllerLocation.Y);
	FVector2D DirXY(Forward.X, Forward.Y);
	if (!DirXY.Normalize())
	{
		return ControllerLocation;
	}

	const float A = DirXY.X * DirXY.X + DirXY.Y * DirXY.Y;
	const float B = 2.f * (DirXY.X * (ControllerXY.X - PlayerXY.X) + DirXY.Y * (ControllerXY.Y - PlayerXY.Y));
	const float C = FMath::Square(ControllerXY.X - PlayerXY.X) + FMath::Square(ControllerXY.Y - PlayerXY.Y) - RadiusValue * RadiusValue;

	const float Discriminant = B * B - 4.f * A * C;
	if (Discriminant < 0.f)
	{
		return ControllerLocation;
	}

	const float SqrtD = FMath::Sqrt(Discriminant);
	const float t1 = (-B - SqrtD) / (2.f * A);
	const float t2 = (-B + SqrtD) / (2.f * A);
	float t = FMath::Min(t1, t2);
	if (t < 0.f)
	{
		t = FMath::Max(t1, t2);
	}
	if (t < 0.f)
	{
		return ControllerLocation;
	}

	FVector Intersection = ControllerLocation + Forward * t;
	return Intersection;
}

ESkillType AStarDrawManager::ResolveSkillFromDraw() const
{
	if (DirectionChoose.Num() == 0)
	{
		return ESkillType::None;
	}

	if (!CachedSkillAsset)
	{
		return ESkillType::None;
	}

	return CachedSkillAsset->GetSkillTypeFromTrail(DirectionChoose);
}
