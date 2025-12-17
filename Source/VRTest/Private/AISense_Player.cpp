// Fill out your copyright notice in the Description page of Project Settings.


#include "AISense_Player.h"
#include "AISenseConfig_Player.h" // needed for digested properties
#include "Perception/AIPerceptionComponent.h" // so we can use the perception system
#include "Kismet/GameplayStatics.h" // so we can have access to GetPlayerPawn()



UAISense_Player::FDigestedPlayerProperties::FDigestedPlayerProperties()
{
	PlayerRadius = 10.f;
	PlayerSightDegree = PI/3;
	bInvisible = false;
	Target_Actor=nullptr;
	Last_Target_Location={0,0,0};

}

UAISense_Player::FDigestedPlayerProperties::FDigestedPlayerProperties(const UAISenseConfig_Player& SenseConfig)
{
	PlayerRadius = SenseConfig.PlayerRadius;
	PlayerSightDegree = SenseConfig.PlayerDegree;
	bInvisible = false;
	Target_Actor=nullptr;
	Last_Target_Location={0,0,0};

}

// inherited initalizer
UAISense_Player::UAISense_Player(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	OnNewListenerDelegate.BindUObject(this, &UAISense_Player::OnNewListenerImpl);
	OnListenerUpdateDelegate.BindUObject(this, &UAISense_Player::OnListenerUpdateImpl);
	OnListenerRemovedDelegate.BindUObject(this, &UAISense_Player::OnListenerRemovedImpl);
}


float UAISense_Player::Update()
{

	// GEngine->AddOnScreenDebugMessage(-1,10,FColor::Red,"init:");
	const UWorld* World = GEngine->GetWorldFromContextObject(GetPerceptionSystem()->GetOuter(), EGetWorldErrorMode::LogAndReturnNull);
	
	if (World == nullptr)
	{
		return SuspendNextUpdate; // defined in the perception component.
	}

	AIPerception::FListenerMap& ListenersMap = *GetListeners();

	// Because we are not using a query system for our perception, we need to get our listerners from our map in another manner
	for (auto& Target : ListenersMap)
	{
		FPerceptionListener& Listener = Target.Value;
		const AActor* LisenerBodyActor = Listener.GetBodyActor();
		// Run Detection event
		// FCollisionShape DetectionSphere = FCollisionShape::MakeSphere(DigestedProperties[Listener.GetListenerID()].PlayerRadius); 
		// TArray<FHitResult> HitResultsLocal;
		// World->SweepMultiByChannel(HitResultsLocal, LisenerBodyActor->GetActorLocation(), LisenerBodyActor->GetActorLocation() + FVector::UpVector * DetectionSphere.GetSphereRadius(), FQuat(), ECollisionChannel::ECC_Pawn, DetectionSphere);
		bool HasCheck=true;
		ACLM_Character* player_character = Cast<ACLM_Character>(UGameplayStatics::GetPlayerPawn(World,0));
		if (player_character == nullptr)
		{
			return 0.0f;
		}
		float distance =(player_character->GetActorLocation() - LisenerBodyActor->GetActorLocation()).Length();
		
		if (distance <= DigestedProperties[Listener.GetListenerID()].PlayerRadius)
		{
		
				
			FVector playerLocation = player_character->GetActorLocation();
			int32 NumberOfLoSChecksPerformed{0};
			float OutSightStrength{1.0f};
			bool hit=player_character->CanBeSeenFrom(Listener.CachedLocation,playerLocation,NumberOfLoSChecksPerformed,OutSightStrength,LisenerBodyActor);
					
			if (hit)
			{
				float multinum=1.0f;
				bool bSeem_temp=CheckTargetInRange(player_character,multinum,Listener);

				if(bSeem_temp)
				{
					Target.Value.RegisterStimulus(player_character, FAIStimulus(*this, multinum, playerLocation, Listener.CachedLocation));
					DigestedProperties[Listener.GetListenerID()].bInvisible = true;
					DigestedProperties[Listener.GetListenerID()].Target_Actor = player_character;
					DigestedProperties[Listener.GetListenerID()].Last_Target_Location=player_character->GetActorLocation();
					HasCheck=false;
				}
			}
		}
		/*for (size_t i = 0; i < HitResultsLocal.Num(); i++)
		{	
			FHitResult HitLocal = HitResultsLocal[i];
			if (HitLocal.GetActor() == UGameplayStatics::GetPlayerPawn(World, 0))
			{
				ACLM_Character* player_character = Cast<ACLM_Character>(HitLocal.GetActor());
					 
				if (player_character == nullptr)
				{
					return 0.0f;
				}
				
				FVector playerLocation = player_character->GetActorLocation();
				int32 NumberOfLoSChecksPerformed{0};
				float OutSightStrength{1.0f};
				bool hit=player_character->CanBeSeenFrom(Listener.CachedLocation,playerLocation,NumberOfLoSChecksPerformed,OutSightStrength,LisenerBodyActor);
					
				if (hit)
				{
					float multinum=1.0f;
					bool bSeem_temp=CheckTargetInRange(player_character,multinum,Listener);

					if(bSeem_temp)
					{
						Target.Value.RegisterStimulus(HitLocal.GetActor(), FAIStimulus(*this, multinum, playerLocation, Listener.CachedLocation));
						DigestedProperties[Listener.GetListenerID()].bInvisible = true;
						DigestedProperties[Listener.GetListenerID()].Target_Actor = player_character;
						DigestedProperties[Listener.GetListenerID()].Last_Target_Location=player_character->GetActorLocation();
						HasCheck=false;
					}
				}
			}
		}*/
		if(HasCheck)
		{
			if(DigestedProperties[Listener.GetListenerID()].bInvisible)
			{
				DigestedProperties[Listener.GetListenerID()].bInvisible = false;
				Target.Value.RegisterStimulus(DigestedProperties[Listener.GetListenerID()].Target_Actor, FAIStimulus(*this, -1, DigestedProperties[Listener.GetListenerID()].Last_Target_Location, Listener.CachedLocation,FAIStimulus::SensingFailed));
			}
		}
	}
	return 0.0f;
}

void UAISense_Player::OnNewListenerImpl(const FPerceptionListener& NewListener)
{
	// Establish lister and sense
	UAIPerceptionComponent* NewListenerPtr = NewListener.Listener.Get();
	check(NewListenerPtr);
	const UAISenseConfig_Player* SenseConfig = Cast<const UAISenseConfig_Player>(NewListenerPtr->GetSenseConfig(GetSenseID()));
	check(SenseConfig);
	// Consume properties
	FDigestedPlayerProperties PropertyDigest(*SenseConfig);
	DigestedProperties.Add(NewListener.GetListenerID(), PropertyDigest);
	//绑定属性
	RequestImmediateUpdate(); // optional. If we were using queries you'd strike this for the GenerateQueriesFobrListener() call instead
}

void UAISense_Player::OnListenerUpdateImpl(const FPerceptionListener& UpdatedListener)
{
}

void UAISense_Player::OnListenerRemovedImpl(const FPerceptionListener& RemovedListener)
{
}


bool UAISense_Player::CheckTargetInRange(ACLM_Character* InTarget, float& multinum, FPerceptionListener& Listener)
{
	float warning_sight;
	
	FVector playerLocation=InTarget->GetActorLocation();
	const AActor* LisenerBodyActor = Listener.GetBodyActor();
	FVector Diraction =playerLocation-LisenerBodyActor->GetActorLocation();
	float warning_sight_degree =DigestedProperties[Listener.GetListenerID()].PlayerSightDegree;
	if(InTarget->bIsInGrass)
	{
		warning_sight=150.0f;
	}
	else
	{
		warning_sight = DigestedProperties[Listener.GetListenerID()].PlayerRadius;

	}
	float Check_Angle = FMath::Acos(FVector::DotProduct(Diraction.GetSafeNormal(), LisenerBodyActor->GetActorForwardVector()));
	
	//前方
	if(Check_Angle <= warning_sight_degree && Diraction.Size() <= warning_sight){
		multinum = (-9.0f*FMath::Square((playerLocation-Listener.CachedLocation).Size()/warning_sight)+10)*FMath::Cos(Check_Angle);
		//调用一次敌人的ui函数，并且传入一个参数
		//不同条件执行不同函数计算参数
		return true;
	}
	//侧后方
	if((PI/2 <= Check_Angle) && Check_Angle<=PI*5/6 && Diraction.Size() <= 150.0f){
		multinum=4.0f;
		return true;
	}
	
	//正后方
	if(PI*5/6 < Check_Angle && Diraction.Size() <= 150.0f){
		multinum=2.0f;
		return true;
	}
	
	//侧前方
	if( warning_sight_degree < Check_Angle && Check_Angle < PI/2 && Diraction.Size() <= warning_sight / 2){
		multinum = (-9.0f*FMath::Square((playerLocation-Listener.CachedLocation).Size()/warning_sight)+10)*FMath::Cos(Check_Angle);
		return true;
	}
	
return false;
}
