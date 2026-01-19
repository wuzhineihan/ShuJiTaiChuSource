// Fill out your copyright notice in the Description page of Project Settings.

#include "Tools/GameUtils.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Game/CollisionConfig.h"

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

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return Results;
	}

	FCollisionObjectQueryParams ObjectQueryParams;
	for (const TEnumAsByte<EObjectTypeQuery>& ObjectType : ObjectTypes)
	{
		ObjectQueryParams.AddObjectTypesToQuery(UEngineTypes::ConvertToCollisionChannel(ObjectType));
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(FindActorsInCone), false);
	QueryParams.AddIgnoredActors(IgnoreActors);

	TArray<FOverlapResult> Overlaps;
	const bool bHit = World->OverlapMultiByObjectType(
		Overlaps,
		Origin,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(Radius),
		QueryParams
	);

	if (!bHit || Overlaps.Num() == 0)
	{
		return Results;
	}

	FVector NormalizedDirection = Direction.GetSafeNormal();
	TSet<AActor*> VisitedActors;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Actor = Overlap.GetActor();
		if (!Actor || VisitedActors.Contains(Actor))
		{
			continue;
		}
		VisitedActors.Add(Actor);

		FVector ToTarget = (Actor->GetActorLocation() - Origin).GetSafeNormal();
		float DotProduct = FVector::DotProduct(NormalizedDirection, ToTarget);
		float Angle = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

		if (Angle <= MaxAngleDegrees)
		{
			Results.Add(FActorWithAngle(Actor, Angle));
		}
	}

	Results.Sort([](const FActorWithAngle& A, const FActorWithAngle& B)
	{
		return A.Angle < B.Angle;
	});

	return Results;
}