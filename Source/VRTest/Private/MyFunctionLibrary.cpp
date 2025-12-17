// Fill out your copyright notice in the Description page of Project Settings.


#include "MyFunctionLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "VectorTypes.h"
#include "Enemy_Base.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

void UMyFunctionLibrary::DestroyAIController(AAIController* AIControllerRef)
{
	AIControllerRef->UnPossess();
	AIControllerRef->Destroy();
}

TArray<FVector> UMyFunctionLibrary::StarCalculateAdjacentPoints(FVector PlayerLocation,
	FVector StarLocation,float LineLength)
{
	//This commet is for the submit test
	TArray<FVector> AdjacentPoints;
	PlayerLocation.Z=StarLocation.Z;
	//calculate Zita and convert it from radians to degrees
	FVector OriginalVector = StarLocation-PlayerLocation;
	float Radius=UE::Geometry::Length(OriginalVector);
	float Zita=UKismetMathLibrary::Acos((2*Radius*Radius-LineLength*LineLength)/(2*Radius*Radius));
	Zita=FMath::RadiansToDegrees(Zita);
	FVector LeftVector = OriginalVector.RotateAngleAxis(Zita,FVector(0,0,1));
	FVector RightVector = OriginalVector.RotateAngleAxis((-1)*Zita,FVector(0,0,1));
	FVector LeftPoint = PlayerLocation+LeftVector;
	FVector RightPoint = PlayerLocation+RightVector;

	FVector UpVector = FVector(0,0,LineLength);
	FVector DownVector = FVector(0,0,-1*LineLength);
	
	FVector UpPoint = StarLocation+UpVector;
	FVector DownPoint = StarLocation+DownVector;
	FVector LeftUpPoint = LeftPoint+UpVector;
	FVector RightUpPoint = RightPoint+UpVector;
	FVector LeftDownPoint = LeftPoint+DownVector;
	FVector RightDownPoint = RightPoint+DownVector;

	AdjacentPoints.Add(LeftPoint);
	AdjacentPoints.Add(RightPoint);
	AdjacentPoints.Add(UpPoint);
	AdjacentPoints.Add(DownPoint);
	AdjacentPoints.Add(LeftUpPoint);
	AdjacentPoints.Add(RightUpPoint);
	AdjacentPoints.Add(LeftDownPoint);
	AdjacentPoints.Add(RightDownPoint);
	
	return AdjacentPoints;
}

float UMyFunctionLibrary::CalculateRadius(FVector PlayerLocation, FVector ControllerLocation,
	FVector ControllerForwardVector, float Distance)
{
	FVector ControllerForwardVectorFlat = ControllerForwardVector;
	ControllerForwardVectorFlat.Z=0;
	ControllerForwardVectorFlat.Normalize();
	FVector TargetLocation = ControllerLocation+ControllerForwardVectorFlat*Distance;
	float Radius = (TargetLocation-PlayerLocation).Size();
	return Radius;
}

FVector UMyFunctionLibrary::CalculateProjectionPoint(FVector PlayerLocation, FVector ControllerLocation, FVector ControllerForwardVector, float Radius)
{
	// 提取XY平面分量
	FVector2D PlayerXY(PlayerLocation.X, PlayerLocation.Y);
	FVector2D ControllerXY(ControllerLocation.X, ControllerLocation.Y);
	FVector2D DirectionXY(ControllerForwardVector.X, ControllerForwardVector.Y);

	// 计算二次方程的系数
	float A = DirectionXY.X * DirectionXY.X + DirectionXY.Y * DirectionXY.Y;
	float B = 2 * (DirectionXY.X * (ControllerXY.X - PlayerXY.X) + DirectionXY.Y * (ControllerXY.Y - PlayerXY.Y));
	float C = FMath::Square(ControllerXY.X - PlayerXY.X) + FMath::Square(ControllerXY.Y - PlayerXY.Y) - Radius * Radius;

	// 计算判别式
	float Discriminant = B * B - 4 * A * C;
	if (Discriminant < 0)
	{
		// 没有交点
		return FVector::ZeroVector;
	}

	// 计算最小的正 t 值
	float t1 = (-B - FMath::Sqrt(Discriminant)) / (2 * A);
	float t2 = (-B + FMath::Sqrt(Discriminant)) / (2 * A);
	float t = FMath::Min(t1, t2);
	if (t < 0)
	{
		t = FMath::Max(t1, t2); // 如果 t1 和 t2 都为负，则取较大的值
	}

	if (t < 0)
	{
		// 交点在光线反方向，无交点
		return FVector::ZeroVector;
	}

	// 计算交点
	FVector Intersection = ControllerLocation + ControllerForwardVector * t;
	return Intersection;
}

void UMyFunctionLibrary::FindNearestEnemy(UObject* WorldContextObject, FVector Location)
{
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	FCollisionShape DetectionSphere = FCollisionShape::MakeSphere(1500.f); 
	TArray<FHitResult> HitResultsLocal;
	float MinDistance = FLT_MAX;
	size_t idx = 0;
	World->SweepMultiByChannel(HitResultsLocal, Location, Location + FVector::UpVector * DetectionSphere.GetSphereRadius(), FQuat(), ECollisionChannel::ECC_Pawn, DetectionSphere);	
	for(size_t i = 0; i < HitResultsLocal.Num(); i++)
	{
		AEnemy_Base* enemy_instance=Cast<AEnemy_Base>(HitResultsLocal[i].GetActor());
		if(enemy_instance==nullptr)
		{
			continue;
		}
		float distance=FVector::Dist(Location,enemy_instance->GetActorLocation());
		if(distance<=MinDistance)
		{
			MinDistance=distance;
			idx=i;
		}
	}
	if(MinDistance==FLT_MAX)
	{
		return;
	}
	AEnemy_Base* nearest_enemy=Cast<AEnemy_Base>(HitResultsLocal[idx].GetActor());
	nearest_enemy->Can_Hear=true;
}

bool UMyFunctionLibrary::IsPositionReachable(UObject* WorldContextObject,FVector TargetPosition, FVector EnemyLocation)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetNavigationSystem(World);
	UNavigationPath* NavPath = NavSys->FindPathToLocationSynchronously(World, EnemyLocation,TargetPosition, NULL);
	if (!NavPath)
		return false;

	return !NavPath->IsPartial();
}

