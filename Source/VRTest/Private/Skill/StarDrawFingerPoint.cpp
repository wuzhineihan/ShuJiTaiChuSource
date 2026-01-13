// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/StarDrawFingerPoint.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Skill/StarDrawManager.h"

AStarDrawFingerPoint::AStarDrawFingerPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SetRootComponent(SphereCollision);
	SphereCollision->SetSphereRadius(5.f);
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollision->SetCollisionResponseToAllChannels(ECR_Overlap);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(SphereCollision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AStarDrawFingerPoint::BeginPlay()
{
	Super::BeginPlay();

	if (SphereCollision)
	{
		SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AStarDrawFingerPoint::OnFingerOverlapBegin);
	}
}

void AStarDrawFingerPoint::SetFingerWorldLocation(const FVector& NewLocation)
{
	SetActorLocation(NewLocation);
}

void AStarDrawFingerPoint::OnFingerOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!DrawManager || !OtherActor)
	{
		return;
	}

	DrawManager->NotifyFingerTouchActor(OtherActor);
}
