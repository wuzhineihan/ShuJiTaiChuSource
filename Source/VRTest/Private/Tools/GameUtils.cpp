// Fill out your copyright notice in the Description page of Project Settings.

#include "Tools/GameUtils.h"
#include "Kismet/KismetSystemLibrary.h"

TArray<FActorWithAngle> UGameUtils::FindActorsInCone(
	UObject* WorldContextObject,
	const FVector& Origin,
	const FVector& Direction,
	float Radius,
	float MaxAngleDegrees,
	const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
	const TArray<AActor*>& IgnoreActors)
{
	TArray<FActorWithAngle> Results;

	if (!WorldContextObject)
	{
		return Results;
	}

	TArray<AActor*> OverlapActors;

	// 球形检测
	bool bHit = UKismetSystemLibrary::SphereOverlapActors(
		WorldContextObject,
		Origin,
		Radius,
		ObjectTypes,
		nullptr,  // 不限制类型
		IgnoreActors,
		OverlapActors
	);

	if (!bHit || OverlapActors.Num() == 0)
	{
		return Results;
	}

	FVector NormalizedDirection = Direction.GetSafeNormal();

	// 计算所有 Actor 的角度
	for (AActor* Actor : OverlapActors)
	{
		if (!Actor)
		{
			continue;
		}

		FVector ToTarget = (Actor->GetActorLocation() - Origin).GetSafeNormal();
		float DotProduct = FVector::DotProduct(NormalizedDirection, ToTarget);
		float Angle = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

		// 只添加在角度范围内的 Actor
		if (Angle <= MaxAngleDegrees)
		{
			Results.Add(FActorWithAngle(Actor, Angle));
		}
	}

	// 按角度从小到大排序
	Results.Sort([](const FActorWithAngle& A, const FActorWithAngle& B)
	{
		return A.Angle < B.Angle;
	});

	return Results;
}

