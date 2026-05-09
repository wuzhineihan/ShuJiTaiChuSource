// Fill out your copyright notice in the Description page of Project Settings.


#include "ChapterThree/CartBase.h"


FName ACartBase::StaticMeshComponentName = "CartMesh";
// Sets default values
ACartBase::ACartBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(ACartBase::StaticMeshComponentName);
	RootComponent = StaticMeshComponent;
	
	
#if WITH_EDITORONLY_DATA
	Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	if (Arrow)
	{
		Arrow->ArrowColor = FColor::Red;
		Arrow->SetSimulatePhysics(false);
		Arrow->SetupAttachment(StaticMeshComponent);
	}
#endif
}




// Called when the game starts or when spawned
void ACartBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACartBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	for (USceneComponent* ChasePoint : ChasePoints)
	{
		if (IsValid(ChasePoint))
		{
			ChasePoint->DestroyComponent();
		}
	}

	for (USphereComponent* DebugPoint : DebugPoints)
	{
		if (IsValid(DebugPoint))
		{
			DebugPoint->DestroyComponent();
		}
	}

	ChasePoints.Empty();
	DebugPoints.Empty();
	AssignedPoints.Empty();
	
	float PositionY = ChasePointSpacing * (ChasePointNums - 1) / 2;
	float PositionX = 0.f;
	float PositionZ = 88.f;
	
	for (int i = 0; i < ChasePointNums; i++)
	{
		PositionX = ChasePointDistance + ChasePointOffset * FMath::FRandRange(-1.f,1.f);
		FString Name = FString("ChasePoint_") + FString::FromInt(i + 1);
		
		USceneComponent* NewChasePoint = NewObject<USceneComponent>(this,*Name);
		NewChasePoint->AttachToComponent(RootComponent,FAttachmentTransformRules::KeepRelativeTransform);
		NewChasePoint->SetRelativeLocation(FVector(PositionX,PositionY,PositionZ));
		NewChasePoint->RegisterComponent();

		USphereComponent* DebugPoint = NewObject<USphereComponent>(this);
		DebugPoint->SetSphereRadius(25);
		DebugPoint->AttachToComponent(NewChasePoint,FAttachmentTransformRules::KeepRelativeTransform);
		DebugPoint->SetVisibility(true);
		DebugPoint->SetHiddenInGame(false);
		DebugPoint->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		DebugPoint->RegisterComponent();
		DebugPoint->SetMobility(EComponentMobility::Movable);
		DebugPoints.Add(DebugPoint);
		
		
		ChasePoints.Add(NewChasePoint);
		PositionY -= ChasePointSpacing;
	}
	
}

FVector ACartBase::GetChasePointLocation(int number)
{
	return ChasePoints[number]->GetComponentLocation();
}

USceneComponent* ACartBase::GetChasePoint(int number)
{
	return ChasePoints[number];
}

int ACartBase::GetChasePointNums()
{
	return ChasePointNums;
}

USceneComponent* ACartBase::AssignChasePoint()
{
	return AcquireChasePoint();
}

void ACartBase::RestitutionChasePoint(USceneComponent* ChasePoint)
{
	ReleaseChasePoint(ChasePoint);
}

bool ACartBase::HasAvailableChasePoints() const
{
	return ChasePoints.Num() > 0;
}

int32 ACartBase::GetAvailableChasePointCount() const
{
	return ChasePoints.Num();
}

USceneComponent* ACartBase::AcquireChasePoint()
{
	if (!HasAvailableChasePoints())
	{
		GEngine->AddOnScreenDebugMessage(-1,10,FColor::Red,"NoMoreChasePoints");
		return nullptr;
	}

	const int32 RandomIndex = FMath::RandHelper(ChasePoints.Num());
	USceneComponent* ChosenChasePoint = ChasePoints[RandomIndex];
	ChasePoints.RemoveAt(RandomIndex);
	AssignedPoints.Add(ChosenChasePoint);
	return ChosenChasePoint;
}

void ACartBase::ReleaseChasePoint(USceneComponent* ChasePoint)
{
	if (!ChasePoint || !AssignedPoints.Contains(ChasePoint))
	{
		return;
	}

	AssignedPoints.Remove(ChasePoint);
	ChasePoints.Add(ChasePoint);
}

// Called every frame
void ACartBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
