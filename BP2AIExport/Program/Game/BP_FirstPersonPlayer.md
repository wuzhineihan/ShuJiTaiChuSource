# BP_FirstPersonPlayer

**Asset Path**: `/Game/Program/Game/BP_FirstPersonPlayer.BP_FirstPersonPlayer`

---

## Metadata

- **Class**: BP_FirstPersonPlayer
- **ParentClass**: BasePlayer

---

## Components

- CollisionCylinder : CapsuleComponent
- Arrow : ArrowComponent (Parent: CollisionCylinder)
- CharMoveComp : CharacterMovementComponent
- CharacterMesh0 : SkeletalMeshComponent (Parent: CollisionCylinder)
- AliveComponent : AliveComponent
- FallDamageComponent : FallDamageComponent
- AutoRecoverComponent : AutoRecoverComponent
- FirstPersonCamera : CameraComponent (Parent: CollisionCylinder)

---

## Variables

(None)

---

## Functions

- UserConstructionScript -> void
- Event BeginPlay (Event) -> void

---

## Graph Inventory

- **Event**: 1
- **Function**: 1

---

## Graph Logic

### [Event] EventGraph

**Trace Start: Event BeginPlay**

```blueprint
* Event BeginPlay
    * (GetSubsystem(EnhancedInput:EnhancedInputLocalPlayerSubsystem) from (GetPlayerController())).AddMappingContext((MappingContext=IMC_FirstPerson, Options=EnhancedInput:ModifyContextOptions(bIgnoreAllPressedKeysUntilRelease=0, bForceImmediately=0, bNotifyUserSettings=0)))
        * [Path ends]
```
---

**Trace Start: EnhancedInputAction IA_FirstPersonMove** (EnhancedInputAction)

```blueprint
* Event EnhancedInputAction IA_FirstPersonMove Args: (ActionValue_X:double, ActionValue_Y:double)
    * Triggered:
        * AddMovementInput((ScaleValue=EnhancedInputAction IA_FirstPersonMove.ActionValue_X, WorldDirection=GetActorRightVector())) (Target: Pawn)
            * AddMovementInput((ScaleValue=EnhancedInputAction IA_FirstPersonMove.ActionValue_Y, WorldDirection=GetActorForwardVector())) (Target: Pawn)
                * [Path ends]
```
---

**Trace Start: EnhancedInputAction IA_FirstPersonLook** (EnhancedInputAction)

```blueprint
* Event EnhancedInputAction IA_FirstPersonLook Args: (ActionValue_X:double, ActionValue_Y:double)
    * Triggered:
        * AddControllerYawInput((Val=EnhancedInputAction IA_FirstPersonLook.ActionValue_X)) (Target: Pawn)
            * AddControllerPitchInput((Val=(EnhancedInputAction IA_FirstPersonLook.ActionValue_Y * -1))) (Target: Pawn)
                * [Path ends]
```
---



---

### [Function] UserConstructionScript

**Trace Start: Construction Script** (FunctionEntry)

```blueprint
* Event Construction Script
    * [Path ends]
```
---

---

## References

- /Game/Program/Game/Input/FirstPersonInput/Actions/IA_FirstPersonLook.IA_FirstPersonLook (InputAction) // EventGraph :: EnhancedInputAction IA_FirstPersonLook.InputAction
- /Game/Program/Game/Input/FirstPersonInput/Actions/IA_FirstPersonMove.IA_FirstPersonMove (InputAction) // EventGraph :: EnhancedInputAction IA_FirstPersonMove.InputAction
- /Game/Program/Game/Input/FirstPersonInput/IMC_FirstPerson.IMC_FirstPerson (InputMappingContext) // EventGraph :: Add Mapping Context.MappingContext
- /Script/EnhancedInput.EnhancedInputLocalPlayerSubsystem (EnhancedInputLocalPlayerSubsystem) // EventGraph :: Get EnhancedInputLocalPlayerSubsystem.ReturnValue
- /Script/EnhancedInput.EnhancedInputSubsystemInterface (EnhancedInputSubsystemInterface) // EventGraph :: Add Mapping Context.self
- /Script/EnhancedInput.InputAction (InputAction) // EventGraph :: EnhancedInputAction IA_FirstPersonMove.InputAction
- /Script/EnhancedInput.InputMappingContext (InputMappingContext) // EventGraph :: Add Mapping Context.MappingContext
- /Script/VRTest.BasePlayer (BasePlayer) // Parent Class

