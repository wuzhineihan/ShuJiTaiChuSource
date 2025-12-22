// Fill out your copyright notice in the Description page of Project Settings.


#include "Jar_Base.h"

#include "Enemy_Base.h"
// Sets default values
AJar_Base::AJar_Base()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AJar_Base::BeginPlay()
{
	Super::BeginPlay();
	
}

void AJar_Base::Find_Nearest_Enemy()
{
	const UWorld* World = GEngine->GetWorldFromContextObject(this->GetWorld(), EGetWorldErrorMode::LogAndReturnNull);
	FCollisionShape DetectionSphere = FCollisionShape::MakeSphere(1500.f); 
	TArray<FHitResult> HitResultsLocal;
	float MinDistance = FLT_MAX;
	size_t idx = 0;
	World->SweepMultiByChannel(HitResultsLocal, this->GetActorLocation(), this->GetActorLocation() + FVector::UpVector * DetectionSphere.GetSphereRadius(), FQuat(), ECollisionChannel::ECC_Pawn, DetectionSphere);	
	for(size_t i = 0; i < HitResultsLocal.Num(); i++)
	{
		AEnemy_Base* enemy_instance=Cast<AEnemy_Base>(HitResultsLocal[i].GetActor());
		if(enemy_instance==nullptr)
		{
			continue;
		}
		float distance=FVector::Dist(this->GetActorLocation(),enemy_instance->GetActorLocation());
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
	// GEngine->AddOnScreenDebugMessage(-1,10,FColor::Red,FString::Printf(TEXT("%f %f %f"),nearest_enemy->GetActorLocation().X,nearest_enemy->GetActorLocation().Y,nearest_enemy->GetActorLocation().Z));
}

// Called every frame
void AJar_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

