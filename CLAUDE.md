# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

VR stealth-combat game with PC (non-VR) and VR dual player modes, built on Unreal Engine 5.4. Currently in an active Blueprint-to-C++ refactoring phase. Content/assets (`.uproject`, Blueprints, maps, meshes) are Perforce-managed and not present in this git worktree — only C++ source and docs are here. Engine installed at `D:\Epic\UE\UE_5.4`.

## Build Commands

All builds use UE's `Build.bat` from the engine directory with a `-waitmutex` flag. The `.uproject` lives at `D:\Perforce\WorkSpace_4090Official\VRTest.uproject`.

```
# Game builds (Win64)
D:\Epic\UE\UE_5.4\Engine\Build\BatchFiles\Build.bat VRTest Win64 Development "D:\Perforce\WorkSpace_4090Official\VRTest.uproject" -waitmutex

# Editor builds (most common for development)
D:\Epic\UE\UE_5.4\Engine\Build\BatchFiles\Build.bat VRTestEditor Win64 Development "D:\Perforce\WorkSpace_4090Official\VRTest.uproject" -waitmutex

# Other configurations: Debug, DebugGame, Test, Shipping
# Clean with Clean.bat (same arguments)
```

Launch: Run `UnrealEditor.exe` with the `.uproject` path as argument. Debug builds use engine binaries at `Engine/Binaries/Win64/UnrealEditor-Win64-Debug.exe`.

## Architecture

### Character Hierarchy

```
ACharacter → ABaseCharacter (IEffectable)
  ├── ABaseEnemy (IGrabbable) → ASacraEnemy (component-composed AI)
  └── ABasePlayer
        ├── ABasePCPlayer  (first-person, ray-based interaction)
        └── ABaseVRPlayer  (motion controllers, grip-based interaction)
```

Key design rule: **PC and VR players are separate subclasses**, not a unified class with mode-switching.

### Core Systems

**Grab System** — Strict grabber/grabblee separation, unified by `UPhysicsHandleComponent`.

- **Grabber side**: `UPlayerGrabHand` (abstract, PhysicsHandle-driven) → `UPCGrabHand` (line trace) / `UVRGrabHand` (sphere trace + gravity glove + backpack).
- **Grabbee side**: `IGrabbable` interface → `AGrabbeeObject` (Free grab) / `AGrabbeeWeapon` (WeaponSnap) / `ABaseEnemy` (HumanBody — death drag).

**Bow/Arrow** — `ABow` supports dual-hand interaction (body via WeaponSnap, string via Custom with spring physics). `AArrow` is a 4-state state machine: Idle → Nocked → Flying → Stuck. Arrow flight uses `PerformFlightTrace()` for per-frame hit detection.

**Effect System** — `IEffectable` interface with composite `FEffect` (can be Arrow+Fire, etc.). `EEffectType`: Arrow, Smash, Melee, Fire, Stasis. `UAliveComponent` manages HP and death.

**Skill System** — Star-draw gesture recognition via `AStarDrawManager` (cylinder projection). `UPlayerSkillComponent` holds learned skills, energy cost, and spawns strategies. `ASkillStrategyBase` subclasses (EagleEye, Shield, Invisible, Freeze, Stasis) implement `Execute()`.

**AI Hatred System** — `USacraEnemyHatredComponent`: 3-state (Idle → Warning → Fight) threat system driven by perception (sight, hearing, damage). Uses `GameplayMessageSubsystem` for cross-enemy communication (alert propagation). Config-driven thresholds and decay timers per state.

**AI GOAP** — A*-based Goal-Oriented Action Planning via `UGoap_Component` + `UGoap_Planner`. Goals (KillEnemy, Patrol, Investigate) and Actions (Attack, EquipWeapon, Search, RequestHelp) are Blueprintable `UObject` subclasses.

**Enemy Weapon** — Polymorphic: `USacraEnemyWeaponComponent` base → `USacraBowWeaponComponent` / `USacraMeleeWeaponComponent`. Animation montage-driven equip/attack cycles with notify-coordinated attack windows.

**Performance** — `USacraEnemySubsystem` manages Heavy/Light phase registration. Distant enemies tick less frequently. Configurable ranges: HeavyRange=5000, LightRange=2500.

### Communication Patterns

- **EventBusComponent** — Local, `FGameplayTag`-keyed event bus within an actor.
- **GameplayMessageSubsystem** — Global, tag-based broadcast/listen between unconnected actors. Used heavily for AI cross-communication.
- **Delegates** — Standard UE multicast delegates for component-to-owner communication.

### Configuration

- `UGameSettings` (DeveloperSettings) — Central project config: BowClass, ArrowClass, SkillAsset, audio, post-process.
- `USacraEnemyConfigDataAsset` — Data asset driving all AI sub-configs (hatred, loadout, context, UI, controller).

## Interactions with Perforce/Content

Content files (Blueprints, maps, meshes, animations) are in the Perforce workspace at `D:\Perforce\WorkSpace_4090Official\` and referenced by class paths in C++. When writing C++ that references Blueprint assets, use `TSubclassOf` or `TSoftObjectPtr` with config-driven defaults rather than hardcoded paths. The `.uproject` and `Plugins/` content are also Perforce-managed — changes to `.uplugin` or module registration must be reflected in the Perforce workspace.

## Code Conventions

- Interfaces are prefixed with `I` (e.g., `IEffectable`, `IGrabbable`) and implemented via `UINTERFACE` + `I*` pair.
- All new components follow the `UActorComponent` subclass pattern. Avoid adding logic directly to `ACharacter` subclasses — compose via components.
- AI types are prefixed with `Sacra` to distinguish from legacy types (e.g., `AEnemy_Base`, `ACLM_Character`).
- Player-facing types distinguish PC vs VR with prefix: `PC*` / `VR*`.

## Docs

- `Docs/GamePlayDesign.txt` — Game design document (Chinese).
- `Docs/Collision/` — Collision channel and profile specifications.
- `Docs/Refactor/AI/` — AI refactoring plans (Obsidian markdown).
- `Docs/Memory.md` — Project memory snapshot (may be stale; verify against current code).
