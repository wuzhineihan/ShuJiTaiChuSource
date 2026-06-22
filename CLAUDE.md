# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

VR stealth-combat game with PC (non-VR) and VR dual player modes, built on **Unreal Engine 5.4**. An active Blueprint-to-C++ refactoring phase is ongoing. The project uses dual-VCS management:
- **Git**: C++ source code, docs, project configuration files (`Config/`, `Source/`, `Plugins/` game code, `Docs/`)
- **Perforce**: Content assets (`.uasset`, `.umap`, Blueprints, meshes, animations), `.uproject`, `Plugins/*/Binaries/` and `Plugins/*/Content/`

P4 workspace at `C:\Users\ArtTech\Documents\DevelopProjects\Developing\Shuji\` (also this git root). Engine at `C:\Program Files\Epic Games\UE_5.4`.

**.gitignore** excludes everything Perforce manages: `Content/`, `Config/`, `*.uproject`, `Plugins/*/Intermediate/`, `Plugins/*/Binaries/`, `Plugins/*/Content/`, `Binaries/`, `Intermediate/`, `Saved/`, `Docs/`.

**.p4ignore** mirrors this split: only tracks `Config/`, `Content/`, `Plugins/` (excluding `Intermediate/` and `Binaries/`), `*.uproject`.

## Build & Launch

```
# All commands use the current workspace path
C:\Program Files\Epic Games\UE_5.4\Engine\Build\BatchFiles\Build.bat VRTestEditor Win64 Development "C:\Users\ArtTech\Documents\DevelopProjects\Developing\Shuji\VRTest.uproject" -waitmutex

# Game build (final package)
C:\Program Files\Epic Games\UE_5.4\Engine\Build\BatchFiles\Build.bat VRTest Win64 Development "C:\Users\ArtTech\Documents\DevelopProjects\Developing\Shuji\VRTest.uproject" -waitmutex

# Clean build
C:\Program Files\Epic Games\UE_5.4\Engine\Build\BatchFiles\Clean.bat VRTestEditor Win64 Development "C:\Users\ArtTech\Documents\DevelopProjects\Developing\Shuji\VRTest.uproject" -waitmutex

# Launch editor
"C:\Program Files\Epic Games\UE_5.4\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Users\ArtTech\Documents\DevelopProjects\Developing\Shuji\VRTest.uproject"

# Generate project files (when .sln is missing/stale)
C:\Program Files\Epic Games\UE_5.4\Engine\Build\BatchFiles\GenerateProjectFiles.bat -project="%cd%\VRTest.uproject" -game -engine
```

If the editor crashes on startup loading a map (CDO serialization mismatch after compiler change), add to `Config/DefaultEngine.ini`:
```ini
[/Script/EngineSettings.GameMapsSettings]
EditorStartupMap=
```
This skips auto-loading and lets you open maps manually.

## Architecture

### Character Hierarchy

```
ACharacter → ABaseCharacter (IEffectable)
  ├── ABaseEnemy (IGrabbable) → ASacraEnemy (component-composed AI)
  └── ABasePlayer
        ├── ABasePCPlayer  (first-person, ray-based interaction)
        └── ABaseVRPlayer  (motion controllers, grip-based interaction)
```

**Key design rule**: PC and VR players are separate subclasses, not a unified class with mode-switching. `ABasePCPlayer` extends `ABasePlayer` with first-person camera and line-trace interaction; `ABaseVRPlayer` uses VR motion controllers and sphere-trace/gravity-glove interaction.

**Game Mode**: `AShujiGameMode` holds separate `VRPawnClass` / `PCPawnClass` and selects via `GetDefaultPawnClassForController_Implementation()`.

### Core Systems

**Grab System** (`Source/.../Grabber/`, `Source/.../Grabbee/`)
- `EGrabType`: None, Free (physics-follow), WeaponSnap (alignment snap), HumanBody (death drag), Custom (complete override).
- **Grabber**: `UPlayerGrabHand` (abstract, PhysicsHandle-driven) → `UPCGrabHand` (line trace, bow mode filtering) / `UVRGrabHand` (sphere trace + gravity glove + backpack storage). Each hand has per-grab-type stiffness/damping config.
- **Grabbee**: `IGrabbable` interface → `AGrabbeeObject` (Free), `AGrabbeeWeapon` (WeaponSnap, sets weapon type for offset), `ABaseEnemy` (HumanBody), `ABreakableGrabbeeObject`.
- [CollisionConfig.h](Source/VRTest/Public/Game/CollisionConfig.h) centralizes all object/trace channel aliases and collision profile names. **Always use these defines** (e.g. `TCC_GRAB`, `OCC_PLAYER_HAND`, `CP_PLAYER_CAPSULE`) instead of raw channel numbers or string literals.

**Bow/Arrow** (`Source/.../Grabbee/`)
- `ABow`: Dual-hand interaction. Body held via WeaponSnap, string via Custom with spring physics (`SpringSolve()`). String has collision overlap detection for VR hand entry and glow effect. PC mode can force string grab via `TryHandleStringHandEnter()`. Arrow trajectory preview via Niagara (`ArrowTracePreview`).
- `AArrow`: 4-state machine (Idle → Nocked → Flying → Stuck). Per-frame `PerformFlightTrace()` hit detection (`ArrowTipPosition` line trace). Supports fire ignition (`CatchFire()`/`Extinguish()`) with Niagara effect and timed duration. Has `IEffectable` for surface interaction, damage on hit, bone-name attachment for ragdoll hits.

**Effect System** (`Source/.../Effect/`)
- `IEffectable` interface with composite `FEffect` struct: array of `EEffectType` (Arrow, Smash, Melee, Fire, Stasis) + Amount + Duration + Causer/Instigator.
- `UAliveComponent`: HP management, death delegate (`FOnDead`), `DecreaseHP`/`IncreaseHP`.
- `UFallDamageComponent`, `UAutoRecoverComponent`: supplementary health systems on `ABasePlayer`.

**Skill System** (`Source/.../Skill/`)
- Gesture recognition via `AStarDrawManager` (cylinder projection, 8-direction star-draw). Directions in `EStarDrawDirection` (Left/Right/Up/Down/LeftUp/RightUp/LeftDown/RightDown) with `StarDrawDirectionGetOpposite()` helper.
- `UPlayerSkillComponent`: learned skills `TSet<ESkillType>`, energy points (cost system), `StartStarDraw()` / `FinishStarDraw()` entry points, strategy instance cache.
- Strategy pattern: `ASkillStrategyBase` subclasses (EagleEye, Shield, Invisible, Freeze, Stasis + Stasis subsystem) implement `Execute()`, spawned on demand. Config driven by `USkillAsset` DataAsset via `UGameSettings`.

**AI Hatred System** (`Source/.../AI/Component/`)
- `USacraEnemyHatredComponent`: 3-state (Idle → Warning → Fight) threat FSM. Driven by perception mods (sight, hearing, damage via gameplay tags). Uses `UGameplayMessageSubsystem` for cross-enemy alert propagation. Config-driven thresholds and decay timers per state (`USacraEnemyHatredDataAsset`).

**AI GOAP** (`Source/.../Goap/`)
- A*-based Goal-Oriented Action Planning via `UGoap_Component` + `UGoap_Planner`. Core types: `UGoap_WorldModel`, `UGoap_WorldState`, `UGoap_PlanGoal`, `UGoap_PlanAction` (all Blueprintable UObject subclasses). Goals: KillEnemy, Patrol, Investigate. Actions: Attack, EquipMeleeWeapon, EquipRangeWeapon, Patrol, PrepareToMeleeAttack, PrepareToRangeAttack, RequestHelp, SearchLocation, Stand.

**Enemy Weapon** (`Source/.../AI/Component/`)
- Polymorphic: `USacraEnemyWeaponComponent` base → `USacraBowWeaponComponent` (ranged, AI bow actor + arrow projectile) / `USacraMeleeWeaponComponent` (melee trigger window). Animation montage-driven equip/attack cycles with notifies (`SacraWeaponAnimNotifies`). Equip state machine via `USacraEnemyLoadoutComponent`.

**AI Behavior Tree** (`Source/.../AI/BehaviorTree/`)
- Organized by state (`Idle/`, `Warning/`, `Fight/`) then by common tasks/services/decorators. Three-state BT structure matching Hatred states. Custom `SacraBlackboardComponent` for blackboard extensions. Custom `AISense_Player` / `AISenseConfig_Player` for player-specific AI perception.

**Climb System** (`Source/.../Scene/`, `Source/.../Game/`)
- `UPlayerClimbComponent` on `ABasePlayer`. Climb detection via `AClimbableVolume`, `ALadderVolumeComponent`, `AWindowVaultVolumeComponent`. PC-specific: `UPCClimbLadderComponent`, `UPCWindowVaultComponent`. VR uses grip-based climb via `UPlayerGrabHand`. Capsule collision changes managed via `CachedCapsuleCollisionProfileBeforeClimb`.

**Audio System** (`Source/.../Audio/`)
- `UAudioSubsystem` (UGameInstanceSubsystem). Tag-driven: `PlayNormalSound2D(FGameplayTag)`, `PlayNormalSoundAtLocation(FGameplayTag, FVector)`. `UAudioNormalSoundAsset` maps FGameplayTag → USoundBase. Global volume multiplier from `UGameSettings`.

**Chapter Three (Carriage Chase)** (`Source/.../ChapterThree/`)
- `ACarriage` / `UCartBase` / `ACartHorseBase` / `AEnemyHorseBase` / `AHorseEnemySpawnManager` — vehicle chase sequence subsystem.
- Steering via `USteeringBehaviourComponent`, animal movement via `UAnimalMovementComponent`.
- Managed by `UChapterThreeManager` and `UCarriageChaseSubsystem`.

**Stasis Skill Subsystem** (`Source/.../Skill/Stasis/`)
- `AStasisSkillStrategy` (Execute → deploy stasis points), `AStasisPoint` (arena effect area), `AFakePhysicsHandleActor`, `AVRStasisFireMonitor` (VR aiming line). `IStasisable` interface for affected objects.

**Performance** — `USacraEnemySubsystem` manages Heavy/Light update phase registration. Distant enemies tick less frequently. Ranges: HeavyRange=5000, LightRange=2500.

### Communication Patterns

| Pattern | Scope | Mechanism |
|---------|-------|-----------|
| `UEventBusComponent` | Local (within one actor) | `FGameplayTag`-keyed `TMap<FGameplayTag, FOnGameplayEventNative>`. Recursive broadcast protection (MAX_BROADCAST_DEPTH=32). Has debug shadow map for editor listener inspection. Supports one-shot listeners. |
| `UGameplayMessageSubsystem` | Global (whole world) | UE's built-in tag-based broadcast between unconnected actors. Used heavily for AI alert propagation (`TAG_AI_Message_Hatred`, `TAG_AI_Message_Weapon_EquipFinished`). |
| Delegates | Component-to-owner | Standard UE multicast delegates (e.g. `FOnObjectGrabbed`, `FOnObjectReleased`). |

### Gameplay Tags

All gameplay tags are declared in [MyProjectTags](Source/VRTest/Public/Game/MyGameplayTags.h) as `UE_DECLARE_GAMEPLAY_TAG_EXTERN()`:
- `TAG_NormalSound_*` — Audio event tags for sound lookup
- `TAG_AI_State_*` — Idle/Warning/Fight state tags and substates (Guard, Patrol)
- `TAG_AI_Behavior_*` — Subtree selection tags for Behavior Tree
- `TAG_AI_Message_*` — Cross-enemy communication messages (Hatred state transitions, weapon events)

### Configuration

- `UGameSettings` (`UDeveloperSettings`, configured in Project Settings → Game → Game Settings): soft class refs for Bow, Arrow, SkillAsset, camera PPM, audio asset, volume multiplier.
- `USacraEnemyConfigDataAsset`: data asset driving all AI sub-configs (hatred, loadout, context, UI, controller).
- `USacraEnemyHatredDataAsset`, `USacraEnemyLoadoutDataAsset`: sub-config data assets.

### Module Dependencies

From `VRTest.Build.cs`:
- `AIModule`, `GameplayTags`, `NavigationSystem`, `HeadMountedDisplay`, `XRBase`
- `UMG`, `Niagara`, `DeveloperSettings`, `GeometryCollectionEngine`, `AssetRegistry`
- `GameplayMessageRuntime` (custom plug-in for global messaging)
- `LevelSequence`, `MovieScene` (cinematics)
- `Slate`, `SlateCore` (editor UI utilities)
- `GameplayDebugger` (private dependency)

### UI / Input

- `PCActionPromptComponent`: screen-space UI prompt manager for PC player actions. `EPCActionPromptType` enum covers all PC actions (StarDraw, ToggleBow, Vault, Ignite, Grab/Release/Throw, Aim, Crouch, etc.).

## Code Conventions

- Interfaces are prefixed with `I` (e.g., `IEffectable`, `IGrabbable`) and implemented via `UINTERFACE` + `I*` pair.
- All new components follow the `UActorComponent` subclass pattern. Avoid adding logic directly to `ACharacter` subclasses — compose via components.
- AI types are prefixed with `Sacra` to distinguish from legacy types (`AEnemy_Base`, `ACLM_Character`) that predate the refactor.
- Player-facing types distinguish PC vs VR with prefix: `PC*` / `VR*`.
- Actor Tags use lower_snake_case (see `SkillTags::OtherStars`).
- Collision defines in `CollisionConfig.h` — use these instead of raw `ECC_GameTraceChannelX` or string profile names.
- Soft references to Blueprint assets (`TSubclassOf`, `TSoftObjectPtr`), never hardcoded paths. Load via `UGameSettings::Get()`.

## Docs

- `Docs/GamePlayDesign.txt` — Game design document (Chinese).
- `Docs/Collision/` — Collision channel and profile specifications.
- `Docs/Refactor/AI/` — AI refactoring plans (Obsidian markdown).
- `Docs/Memory.md` — Project memory snapshot (may be stale; verify against current code).
