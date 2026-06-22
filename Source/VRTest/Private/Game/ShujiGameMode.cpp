// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ShujiGameMode.h"
#include "Game/PCPlayerController.h"
#include "Game/ShujiSaveGame.h"
#include "Skill/PlayerSkillComponent.h"
#include "Game/InventoryComponent.h"
#include "Game/Characters/SacraEnemy.h"
#include "Effect/AliveComponent.h"
#include "IXRTrackingSystem.h"
#include "IHeadMountedDisplay.h"
#include "Game/Characters/BaseVRPlayer.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Player.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "GameMapsSettings.h"
#include "Misc/PackageName.h"

const FString AShujiGameMode::SaveSlotName = TEXT("ShujiSaveSlot");

AShujiGameMode::AShujiGameMode()
{
	PlayerControllerClass = APCPlayerController::StaticClass();
}

UClass* AShujiGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (DefaultPawnClass)
	{
		if (DefaultPawnClass->IsChildOf(ABaseVRPlayer::StaticClass()))
			bIsVRMode = true;
		else
			bIsVRMode = false;
		return DefaultPawnClass;
	}

	if (GEngine && GEngine->XRSystem.IsValid() && GEngine->XRSystem->GetHMDDevice())
	{
		bIsVRMode = GEngine->XRSystem->GetHMDDevice()->IsHMDEnabled();
	}

	if (bIsVRMode)
	{
		if (VRPawnClass)
		{
			return VRPawnClass;
		}
	}
	else
	{
		if (PCPawnClass)
		{
			return PCPawnClass;
		}
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

bool AShujiGameMode::GetIsVRMode()
{
    return bIsVRMode;
}

// ==================== PC HUD ====================

void AShujiGameMode::ShowPCHUD()
{
	if (!PCHUDWidgetClass) return;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	if (!PCHUDInstance)
	{
		PCHUDInstance = CreateWidget<UUserWidget>(PC, PCHUDWidgetClass);
	}
	if (PCHUDInstance && !PCHUDInstance->IsInViewport())
	{
		PCHUDInstance->AddToViewport();
		UE_LOG(LogTemp, Log, TEXT("[HUD] ShowPCHUD"));
	}
}

void AShujiGameMode::HidePCHUD()
{
	if (PCHUDInstance && PCHUDInstance->IsInViewport())
	{
		PCHUDInstance->RemoveFromParent();
		UE_LOG(LogTemp, Log, TEXT("[HUD] HidePCHUD"));
	}
}

// ==================== 主菜单系统 ====================

void AShujiGameMode::BeginPlay()
{
	Super::BeginPlay();

	FString CurrentLevelName = GetWorld()->GetMapName();
	CurrentLevelName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
	FString LoadingLevelPackage = FPackageName::ObjectPathToPackageName(LoadingLevel.ToSoftObjectPath().ToString());
	bool bIsLoadingLevel = (CurrentLevelName == FPackageName::GetShortName(LoadingLevelPackage));

	if (bIsLoadingLevel)
	{
		UE_LOG(LogTemp, Log, TEXT("[LoadingLevel] BeginPlay - Showing loading screen."));

		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (!PC) return;

		if (LoadingScreenWidgetClass)
		{
			UUserWidget* Widget = CreateWidget<UUserWidget>(PC, LoadingScreenWidgetClass);
			if (Widget)
			{
				Widget->AddToViewport(10000);
			}
		}

		PC->SetShowMouseCursor(true);
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(InputMode);

		FString MapPath = UGameMapsSettings::GetGameDefaultMap();
		int32 DotIdx = MapPath.Find(TEXT("."));
		if (DotIdx != INDEX_NONE)
		{
			MapPath = MapPath.Left(DotIdx);
		}
		FString PackagePath = MapPath;
		if (FPackageName::IsValidObjectPath(MapPath))
		{
			PackagePath = FPackageName::ObjectPathToPackageName(MapPath);
		}
		else if (!MapPath.StartsWith(TEXT("/")))
		{
			PackagePath = FString::Printf(TEXT("/Game/%s"), *MapPath);
		}

		UE_LOG(LogTemp, Log, TEXT("[LoadingLevel] Async loading target map: %s"), *PackagePath);
		LoadPackageAsync(PackagePath, FLoadPackageAsyncDelegate::CreateLambda([this, PackagePath](const FName& InPackageName, UPackage*, EAsyncLoadingResult::Type)
		{
			if (!IsValid(this)) return;
			UGameplayStatics::OpenLevel(this, FName(*PackagePath));
		}));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[MainMenu] BeginPlay - MenuCameraTag='%s'"), *MenuCameraTag.ToString());

	ACameraActor* MenuCamera = FindMenuCamera();
	if (!MenuCamera)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MainMenu] BeginPlay - No CameraActor with Tag '%s' found, skipping menu setup."), *MenuCameraTag.ToString());
		return;
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MainMenu] BeginPlay - No PlayerController, skipping menu setup."));
		return;
	}

	if (APCPlayerController* ShujiPC = Cast<APCPlayerController>(PC))
	{
		ShujiPC->SetPlayerInputEnabled(false);
	}

	PC->SetViewTarget(MenuCamera);

	if (UCameraComponent* MenuCameraComp = MenuCamera->GetCameraComponent())
	{
		MenuCameraComp->bConstrainAspectRatio = false;
	}

	if (PC->GetPawn())
	{
		PC->DisableInput(PC);
	}

	PC->SetShowMouseCursor(true);
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	PC->SetInputMode(InputMode);

	if (MainMenuWidgetClass)
	{
		MainMenuWidgetInstance = CreateWidget<UUserWidget>(PC, MainMenuWidgetClass);
		if (MainMenuWidgetInstance)
		{
			MainMenuWidgetInstance->AddToViewport();
		}
	}

	HidePCHUD();
}

void AShujiGameMode::StartGame()
{
	UE_LOG(LogTemp, Log, TEXT("[MainMenu] StartGame called! bIsStartingGame=%d"), bIsStartingGame);
	if (bIsStartingGame) return;
	bIsStartingGame = true;

	UWorld* World = GetWorld();
	if (!World) { bIsStartingGame = false; return; }

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC) { bIsStartingGame = false; return; }

	APawn* PlayerPawn = PC->GetPawn();
	if (!PlayerPawn) { bIsStartingGame = false; return; }

	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->RemoveFromParent();
		MainMenuWidgetInstance = nullptr;
	}

	PC->SetShowMouseCursor(false);
	FInputModeGameOnly GameInputMode;
	PC->SetInputMode(GameInputMode);
	PC->SetIgnoreLookInput(true);
	PC->SetIgnoreMoveInput(true);

	PC->SetViewTargetWithBlend(PlayerPawn, CameraBlendDuration, VTBlend_EaseInOut, CameraBlendExponent);

	FTimerHandle BlendTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(BlendTimerHandle, FTimerDelegate::CreateWeakLambda(this, [this, PC]()
	{
		if (!PC || !IsValid(this)) return;

		if (APCPlayerController* ShujiPC = Cast<APCPlayerController>(PC))
		{
			ShujiPC->SetPlayerInputEnabled(true);
		}

		PC->SetIgnoreLookInput(false);
		PC->SetIgnoreMoveInput(false);
		if (PC->GetPawn()) PC->EnableInput(PC);

		bIsStartingGame = false;

		ShowPCHUD();

		OnMenuTransitionComplete.Broadcast();
	}), CameraBlendDuration, false);
}

ACameraActor* AShujiGameMode::FindMenuCamera() const
{
	for (TActorIterator<ACameraActor> It(GetWorld()); It; ++It)
	{
		if (It->ActorHasTag(MenuCameraTag)) return *It;
	}
	return nullptr;
}

// ==================== 存档系统 ====================

void AShujiGameMode::SaveGame()
{
	UShujiSaveGame* SaveData = NewObject<UShujiSaveGame>();

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (APawn* MyPawn = PC ? PC->GetPawn() : nullptr)
	{
		SaveData->PlayerLocation = MyPawn->GetActorLocation();
		SaveData->PlayerRotation = MyPawn->GetActorRotation();

		if (UPlayerSkillComponent* SkillComp = MyPawn->FindComponentByClass<UPlayerSkillComponent>())
		{
			SaveData->bStarDrawEnabled = SkillComp->IsStarDrawEnabled();
			SaveData->LearnedSkills = SkillComp->GetLearnedSkills();
			SaveData->CurrentEnergyPoints = SkillComp->GetCurrentEnergyPoints();
		}

		if (UInventoryComponent* Inventory = MyPawn->FindComponentByClass<UInventoryComponent>())
		{
			SaveData->bHasBow = Inventory->HasBow();
			SaveData->ArrowCount = Inventory->GetArrowCount();
		}
	}

	SaveData->DeadEnemyIDs.Empty();
	for (TActorIterator<ASacraEnemy> It(GetWorld()); It; ++It)
	{
		if (It->bIsDead && !It->EnemyID.IsNone())
		{
			SaveData->DeadEnemyIDs.Add(It->EnemyID);
		}
	}

	UGameplayStatics::SaveGameToSlot(SaveData, SaveSlotName, 0);
	UE_LOG(LogTemp, Log, TEXT("[SaveGame] Saved to slot '%s'"), *SaveSlotName);
}

void AShujiGameMode::LoadGame()
{
	UShujiSaveGame* SaveData = Cast<UShujiSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
	if (!SaveData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SaveGame] No save data in slot '%s'"), *SaveSlotName);
		return;
	}

	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->RemoveFromParent();
		MainMenuWidgetInstance = nullptr;
	}

	if (LoadingScreenWidgetClass)
	{
		LoadingScreenInstance = CreateWidget<UUserWidget>(GetWorld()->GetFirstPlayerController(), LoadingScreenWidgetClass);
		if (LoadingScreenInstance)
		{
			LoadingScreenInstance->AddToViewport(10000);
		}
	}

	const FVector LoadLocation = SaveData->PlayerLocation;
	const FRotator LoadRotation = SaveData->PlayerRotation;
	const bool bLoadStarDraw = SaveData->bStarDrawEnabled;
	const TSet<ESkillType> LoadSkills = SaveData->LearnedSkills;
	const int32 LoadEnergy = SaveData->CurrentEnergyPoints;
	const bool bLoadHasBow = SaveData->bHasBow;
	const int32 LoadArrowCount = SaveData->ArrowCount;
	const TSet<FName> LoadDeadIDs = SaveData->DeadEnemyIDs;

	FTimerHandle LoadTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(LoadTimerHandle, FTimerDelegate::CreateWeakLambda(this,
		[this, LoadLocation, LoadRotation, bLoadStarDraw, LoadSkills, LoadEnergy,
		 bLoadHasBow, LoadArrowCount, LoadDeadIDs]()
		{
			APlayerController* PC = GetWorld()->GetFirstPlayerController();
			if (!PC || !IsValid(this)) return;

			if (APawn* MyPawn = PC->GetPawn())
			{
				MyPawn->SetActorLocation(LoadLocation, false, (FHitResult*)nullptr, ETeleportType::TeleportPhysics);
				MyPawn->SetActorRotation(LoadRotation);

				if (UPlayerSkillComponent* SkillComp = MyPawn->FindComponentByClass<UPlayerSkillComponent>())
				{
					SkillComp->SetStarDrawEnabled(bLoadStarDraw);
					SkillComp->ClearLearnedSkills();
					for (ESkillType Skill : LoadSkills) SkillComp->LearnSkill(Skill);
					SkillComp->SetEnergyPoints(LoadEnergy);
				}

				if (UInventoryComponent* Inventory = MyPawn->FindComponentByClass<UInventoryComponent>())
				{
					Inventory->SetHasBow(bLoadHasBow);
					Inventory->SetArrowCount(LoadArrowCount);
				}
			}

			for (TActorIterator<ASacraEnemy> It(GetWorld()); It; ++It)
			{
				if (LoadDeadIDs.Contains(It->EnemyID))
				{
					if (UAliveComponent* Alive = It->AliveComponent)
					{
						Alive->SetHP(0.0f);
					}
				}
			}

			PC->SetViewTarget(PC->GetPawn());
			PC->SetIgnoreMoveInput(false);
			PC->SetIgnoreLookInput(false);
			if (PC->GetPawn()) PC->EnableInput(PC);
			PC->SetShowMouseCursor(false);
			FInputModeGameOnly InputMode;
			PC->SetInputMode(InputMode);

			if (APCPlayerController* ShujiPC = Cast<APCPlayerController>(PC))
			{
				ShujiPC->SetPlayerInputEnabled(true);
			}

			if (LoadingScreenInstance)
			{
				LoadingScreenInstance->RemoveFromParent();
				LoadingScreenInstance = nullptr;
			}

			ShowPCHUD();

			UE_LOG(LogTemp, Log, TEXT("[SaveGame] Loaded from slot '%s'"), *SaveSlotName);
		}), LoadingScreenMinDuration, false);
}

void AShujiGameMode::ReturnToMainMenu()
{
	UE_LOG(LogTemp, Log, TEXT("[ReturnToMainMenu] Called."));

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (APCPlayerController* ShujiPC = Cast<APCPlayerController>(PC))
		{
			if (ShujiPC->IsGamePaused()) ShujiPC->ResumeGame();
		}
	}

	FString LvlPkg = FPackageName::ObjectPathToPackageName(LoadingLevel.ToSoftObjectPath().ToString());
	UGameplayStatics::OpenLevel(this, FName(*LvlPkg));
}

bool AShujiGameMode::HasSaveData() const
{
	return UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0);
}
