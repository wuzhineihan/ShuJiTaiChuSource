// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/PCWindowVaultComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Game/Characters/BasePCPlayer.h"
#include "Game/CollisionConfig.h"
#include "Game/PCClimbLadderComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Scene/WindowVaultVolumeComponent.h"

UPCWindowVaultComponent::UPCWindowVaultComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetComponentTickEnabled(false);
}

void UPCWindowVaultComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerPlayer = Cast<ABasePCPlayer>(GetOwner());
	if (!OwnerPlayer)
	{
		return;
	}

	OwnerCamera = OwnerPlayer->FirstPersonCamera;
	OwnerCapsule = OwnerPlayer->GetCapsuleComponent();
}

bool UPCWindowVaultComponent::TryStartVaultBySight(UPrimitiveComponent* SightHitComponent)
{
	if (!OwnerPlayer || !OwnerCamera || !OwnerCapsule || bVaulting)
	{
		return false;
	}

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	if (Now - LastVaultEndTime < VaultCooldown)
	{
		return false;
	}

	UWindowVaultVolumeComponent* VaultVolume = Cast<UWindowVaultVolumeComponent>(SightHitComponent);
	if (!VaultVolume)
	{
		return false;
	}

	const FVector CameraLoc = OwnerCamera->GetComponentLocation();
	const FVector VolumeLoc = VaultVolume->GetComponentLocation();
	if (FVector::DistSquared(CameraLoc, VolumeLoc) > FMath::Square(MaxVaultSightDistance))
	{
		return false;
	}

	FVector Start = FVector::ZeroVector;
	FVector Apex = FVector::ZeroVector;
	FVector End = FVector::ZeroVector;
	if (!BuildVaultPath(VaultVolume, Start, Apex, End))
	{
		return false;
	}

	StartVault(VaultVolume, Start, Apex, End);
	return true;
}

void UPCWindowVaultComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bVaulting || !OwnerPlayer || !OwnerCamera)
	{
		return;
	}

	UpdateVault(DeltaTime);
}

bool UPCWindowVaultComponent::BuildVaultPath(UWindowVaultVolumeComponent* VaultVolume, FVector& OutStart, FVector& OutApex, FVector& OutEnd) const
{
	if (!OwnerPlayer || !OwnerCamera || !OwnerCapsule || !VaultVolume)
	{
		return false;
	}

	const FVector CamLoc = OwnerCamera->GetComponentLocation();
	const FVector Center = VaultVolume->GetComponentLocation();
	const FVector Forward = VaultVolume->GetVaultForward().GetSafeNormal2D();
	if (Forward.IsNearlyZero())
	{
		return false;
	}

	const float Offset = VaultVolume->FrontBackOffset;
	const FVector Front = Center + Forward * Offset;
	const FVector Back = Center - Forward * Offset;
	const float SideSign = FVector::DotProduct((CamLoc - Center).GetSafeNormal2D(), Forward);
	const FVector StartXY = SideSign >= 0.0f ? Front : Back;
	const FVector EndXY = SideSign >= 0.0f ? Back : Front;

	OutStart = FVector(StartXY.X, StartXY.Y, CamLoc.Z);

	float GroundZ = 0.0f;
	if (!TraceGroundAtXY(FVector2D(EndXY.X, EndXY.Y), VaultVolume->GroundTraceDistance, GroundZ))
	{
		return false;
	}

	const float CapsuleHalfHeight = OwnerCapsule->GetScaledCapsuleHalfHeight();
	const float CapsuleBottomZ = OwnerCapsule->GetComponentLocation().Z - CapsuleHalfHeight;
	const float CamToBottomOffset = CamLoc.Z - CapsuleBottomZ;
	OutEnd = FVector(EndXY.X, EndXY.Y, GroundZ + CamToBottomOffset);

	const FVector Extent = VaultVolume->GetScaledBoxExtent();
	const double ApexZ = Center.Z + static_cast<double>(VaultVolume->ApexExtraZ);
	OutApex = FVector(Center.X, Center.Y, ApexZ);

	return true;
}

bool UPCWindowVaultComponent::TraceGroundAtXY(const FVector2D& XY, float TraceDistance, float& OutGroundZ) const
{
	OutGroundZ = 0.0f;
	if (!GetWorld() || !OwnerPlayer)
	{
		return false;
	}

	const float StartZ = OwnerPlayer->GetActorLocation().Z;
	const FVector Start(XY.X, XY.Y, StartZ);
	const FVector End(XY.X, XY.Y, StartZ - TraceDistance);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(WindowVaultGroundTrace), false);
	QueryParams.AddIgnoredActor(OwnerPlayer);

	FHitResult Hit;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, QueryParams);
	if (!bHit)
	{
		return false;
	}

	OutGroundZ = Hit.ImpactPoint.Z;
	return true;
}

void UPCWindowVaultComponent::StartVault(UWindowVaultVolumeComponent* VaultVolume, const FVector& Start, const FVector& Apex, const FVector& End)
{
	if (!OwnerPlayer || !OwnerCamera || !OwnerCapsule || !VaultVolume)
	{
		return;
	}

	VaultStart = Start;
	VaultApex = Apex;
	VaultEnd = End;

	UE_LOG(LogTemp, Log, TEXT("[WindowVault] Start Z=%.1f  Apex Z=%.1f  End Z=%.1f  BoxTopZ=%.1f  GroundZ=%.1f  CamToBottom=%.1f"),
		Start.Z, Apex.Z, End.Z,
		VaultVolume->GetComponentLocation().Z + VaultVolume->GetScaledBoxExtent().Z,
		End.Z - CameraToCapsuleBottomOffset,
		CameraToCapsuleBottomOffset);
	DurationToStart = VaultVolume->PreAlignDuration;
	DurationToApex = VaultVolume->ToApexDuration;
	DurationToEnd = VaultVolume->ToLandDuration;

	const float CapsuleHalfHeight = OwnerCapsule->GetScaledCapsuleHalfHeight();
	const float CapsuleBottomZ = OwnerCapsule->GetComponentLocation().Z - CapsuleHalfHeight;
	CameraToCapsuleBottomOffset = OwnerCamera->GetComponentLocation().Z - CapsuleBottomZ;

	PhaseStartCameraLocation = OwnerCamera->GetComponentLocation();
	VaultPhase = EPCWindowVaultPhase::ToStart;
	PhaseElapsed = 0.0f;
	bVaulting = true;
	SetComponentTickEnabled(true);

	if (APlayerController* PlayerController = Cast<APlayerController>(OwnerPlayer->GetController()))
	{
		PlayerController->SetIgnoreMoveInput(true);
		PlayerController->SetIgnoreLookInput(true);
	}
	OwnerPlayer->SetActionLocked(true);
	if (OwnerPlayer->PCClimbLadderComponent)
	{
		OwnerPlayer->PCClimbLadderComponent->SetComponentTickEnabled(false);
	}
	if (UCharacterMovementComponent* MoveComp = OwnerPlayer->GetCharacterMovement())
	{
		MoveComp->SetMovementMode(MOVE_Walking);
	}

	CachedCapsuleCollisionProfile = OwnerCapsule->GetCollisionProfileName();
	OwnerCapsule->SetCollisionProfileName(CP_NO_COLLISION);
}

void UPCWindowVaultComponent::FinishVault()
{
	if (!OwnerPlayer || !OwnerCamera || !OwnerCapsule)
	{
		bVaulting = false;
		VaultPhase = EPCWindowVaultPhase::None;
		SetComponentTickEnabled(false);
		return;
	}

	if (!CachedCapsuleCollisionProfile.IsNone())
	{
		OwnerCapsule->SetCollisionProfileName(CachedCapsuleCollisionProfile);
	}
	else
	{
		OwnerCapsule->SetCollisionProfileName(CP_PLAYER_CAPSULE);
	}
	CachedCapsuleCollisionProfile = NAME_None;

	if (APlayerController* PlayerController = Cast<APlayerController>(OwnerPlayer->GetController()))
	{
		PlayerController->SetIgnoreMoveInput(false);
		PlayerController->SetIgnoreLookInput(false);
	}
	OwnerPlayer->SetActionLocked(false);
	if (OwnerPlayer->PCClimbLadderComponent)
	{
		OwnerPlayer->PCClimbLadderComponent->SetComponentTickEnabled(true);
	}

	bVaulting = false;
	VaultPhase = EPCWindowVaultPhase::None;
	PhaseElapsed = 0.0f;
	SetComponentTickEnabled(false);
	LastVaultEndTime = GetWorld() ? GetWorld()->GetTimeSeconds() : LastVaultEndTime;
}

void UPCWindowVaultComponent::UpdateVault(float DeltaTime)
{
	if (!OwnerPlayer || !OwnerCamera)
	{
		FinishVault();
		return;
	}

	float PhaseDuration = DurationToEnd;
	if (VaultPhase == EPCWindowVaultPhase::ToStart)
	{
		PhaseDuration = DurationToStart;
	}
	else if (VaultPhase == EPCWindowVaultPhase::ToApex)
	{
		PhaseDuration = DurationToApex;
	}
	else if (VaultPhase == EPCWindowVaultPhase::ToEnd)
	{
		PhaseDuration = DurationToEnd;
	}

	PhaseElapsed += DeltaTime;
	const float Alpha = PhaseDuration > 0.0f ? FMath::Clamp(PhaseElapsed / PhaseDuration, 0.0f, 1.0f) : 1.0f;

	const FVector TargetCamera = CurrentTargetForPhase();
	const FVector CurrentCamera = OwnerCamera->GetComponentLocation();
	const FVector DesiredCamera = FMath::Lerp(PhaseStartCameraLocation, TargetCamera, Alpha);
	const FVector ActorDelta = DesiredCamera - CurrentCamera;

	if (!ActorDelta.IsNearlyZero())
	{
		OwnerPlayer->AddActorWorldOffset(ActorDelta, false, nullptr, ETeleportType::TeleportPhysics);
	}

	if (bDrawDebug)
	{
		DrawDebugSphere(GetWorld(), VaultStart, 8.0f, 8, FColor::Blue, false, 0.01f);
		DrawDebugSphere(GetWorld(), VaultApex, 8.0f, 8, FColor::Cyan, false, 0.01f);
		DrawDebugSphere(GetWorld(), VaultEnd, 8.0f, 8, FColor::Green, false, 0.01f);
	}

	if (Alpha >= 1.0f - KINDA_SMALL_NUMBER)
	{
		AdvancePhase();
	}
}

void UPCWindowVaultComponent::AdvancePhase()
{
	PhaseElapsed = 0.0f;
	PhaseStartCameraLocation = OwnerCamera ? OwnerCamera->GetComponentLocation() : FVector::ZeroVector;

	if (VaultPhase == EPCWindowVaultPhase::ToStart)
	{
		VaultPhase = EPCWindowVaultPhase::ToApex;
		return;
	}
	if (VaultPhase == EPCWindowVaultPhase::ToApex)
	{
		VaultPhase = EPCWindowVaultPhase::ToEnd;
		return;
	}

	FinishVault();
}

FVector UPCWindowVaultComponent::CurrentTargetForPhase() const
{
	if (VaultPhase == EPCWindowVaultPhase::ToStart)
	{
		return VaultStart;
	}
	if (VaultPhase == EPCWindowVaultPhase::ToApex)
	{
		return VaultApex;
	}
	return VaultEnd;
}
