# Code Summary: VR Stealth Combat Game

This document provides a high-level overview of the codebase for the VR Stealth Combat Game, focusing on the VRGameplay logic. It is intended to serve as a reference for developers working on the project.

## 1. Core Architecture

The project uses a hybrid C++ and Blueprint architecture. Core character logic and performance-critical systems are implemented in C++, while gameplay logic, VR interactions, and skill systems are implemented in Blueprints.

### 1.1 C++ Classes (`Source/VRTest`)
*   **`ACLM_Character`**: The base character class inheriting from `ACharacter` and implementing `IAISightTargetInterface`.
    *   **Responsibilities**: Handles basic character movement capabilities and stealth-related properties (e.g., `bIsInGrass`, `CanBeSeenFrom`). It serves as the parent class for the VR Pawn.

### 1.2 Blueprint Classes (`BP2AIExport/VRGamePlay`)
*   **`BP_VRPawn`**: The main player character, inheriting from `ACLM_Character`.
    *   **Responsibilities**:
        *   **VR Setup**: Manages Camera, Motion Controllers, and VROrigin.
        *   **State Management**: Tracks health (`PlayerHealth`), climbing state (`IsClimbing`), and drawing state (`IsDrawing`).
        *   **Inventory**: Manages the Backpack system (`BackPack`, `BowInBackPack`, `ArrowNumber`).
        *   **Manager Orchestration**: Owns and initializes `BP_DrawManager` and `BP_SkillReleaseManager`.
*   **`BP_VRPlayerState`**: Manages persistent player data.
    *   **Responsibilities**: Tracks learned skills (`SkillLearnedMap`), magic points (`Magic`), and inventory state (`ArrowNumber`, `BowInBackPack`).

## 2. VR Interaction System

The VR interaction system is built around custom motion controllers and grab components.

*   **`BP_MyMotionController`**: A custom component attached to the VR Pawn's hands.
    *   **Responsibilities**:
        *   **Grabbing**: Detects and handles grabbing of objects (`MotionControllerGrabEvent`, `MotionControllerReleaseEvent`).
        *   **State**: Tracks what is being held (`HeldComponent`, `IsHolding`).
        *   **Interaction**: Interacts with `BP_MyGrabComponent` on actors.
*   **`BP_MyGrabComponent`**: A component added to any actor that needs to be interactable (grabbable).
    *   **Responsibilities**: Defines how an object behaves when grabbed.
*   **`BP_VRFunctionLibrary`**: Contains utility functions, such as `GetHeldByHand` to determine which hand is holding an object.

## 3. Combat System

The combat system focuses on archery and physics-based interactions.

*   **`BP_WeaponParent`**: Base class for weapons, implementing `BPI_WeaponInfo`.
*   **`BP_Bow`**: The primary weapon.
    *   **Responsibilities**: Handles string pulling mechanics (`Spring Solve` macro), arrow spawning (`SpawnArrow`), and audio feedback.
*   **`BP_Arrow`**: The projectile used by the bow.
    *   **Responsibilities**: Handles flight physics (`ProjectileMovement`), collision detection (`OnComponentBeginOverlap`), and damage application (`BPI_TakeDamage_New`). It supports states like `Normal`, `PullingString`, and `Fire`.
*   **`BPI_TakeDamage_New`**: Interface for actors that can receive damage.

## 4. Skill System (Star Map & Stasis)

The skill system requires players to draw "Star Maps" in VR to cast spells, and includes a Stasis ability.

*   **`BP_DrawManager`**: Manages the drawing process.
    *   **Responsibilities**: Tracks finger movement, spawns stars (`BP_MainStar`, `BP_OtherStars`), and matches trails to skills.
*   **`BP_SkillReleaseManager`**: Handles the execution of skills (Eagle Eye, Shield, Invisible).
*   **`BP_StasisPoint`**: Represents an object affected by the Stasis skill.
    *   **Responsibilities**: Implements `BPI_Stasisable` and `BPI_CustomGrabAndRelease`. It can enter a "Track Mode" or "Following Mode" using physics handles.
*   **`BP_WorldDynamicObject` / `BP_WorldDynamic_Grabbable`**: Base classes for objects that can be affected by Stasis.

## 5. Climbing & Locomotion

*   **`BP_ClimbableVolume`**: An actor that defines a climbable area.
    *   **Responsibilities**: Implements `BPI_CustomGrabAndRelease` to move the player based on hand movement.

## 6. Environment & Systems

*   **Save/Load**: `BP_SaveManager` handles saving and loading game state using `BP_SaveGame`. `BP_CheckPoint` triggers auto-saves.
*   **Beginner's Guide**: `BP_BeginnersGuideManager` controls the tutorial flow, highlighting controller buttons (`ButtonStartFlickering`) and managing tutorial triggers (`BP_BeginnersGuideTriggerBox`).
*   **Scene Elements**: Includes interactive elements like Torches (`BP_BigTorch`, `BPC_FireVolume`) which can interact with arrows (setting them on fire).

## 7. Folder Structure Key
*   `Source/VRTest`: C++ Source Code.
*   `BP2AIExport/VRGamePlay/Player`: Player Pawn, Player State, and VR Controller logic.
*   `BP2AIExport/VRGamePlay/Combat`: Weapons (Bow, Arrow) and combat interfaces.
*   `BP2AIExport/VRGamePlay/Skill`: Skill drawing, release logic, and Stasis system.
*   `BP2AIExport/VRGamePlay/Climb`: Climbing system logic.
*   `BP2AIExport/VRGamePlay/SaveAndLoad`: Save system.
*   `BP2AIExport/VRGamePlay/BeginnersGuide`: Tutorial system.
*   `BP2AIExport/VRGamePlay/Scene`: Environmental interactables (Torches).
