# BP_VRPlayer

**Asset Path**: `/Game/Program/Game/BP_VRPlayer.BP_VRPlayer`

---

## Metadata

- **Class**: BP_VRPlayer
- **ParentClass**: BaseCharacter

---

## Components

- CollisionCylinder : CapsuleComponent
- Arrow : ArrowComponent (Parent: CollisionCylinder)
- CharMoveComp : CharacterMovementComponent
- CharacterMesh0 : SkeletalMeshComponent (Parent: CollisionCylinder)
- AliveComponent : AliveComponent

---

## Variables

(None)

---

## Functions

- UserConstructionScript -> void
- Event BeginPlay (Event) -> void
- Event ActorBeginOverlap (Event) -> void ( OtherActor: Actor )
- Event Tick (Event) -> void ( DeltaSeconds: real )

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
    * [Path ends]
```
---

**Trace Start: Event ActorBeginOverlap**

```blueprint
* Event ReceiveActorBeginOverlap Args: (OtherActor:Actor)
    * [Path ends]
```
---

**Trace Start: Event Tick**

```blueprint
* Event Tick Args: (DeltaSeconds:float)
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

- /Script/VRTest.BaseCharacter (BaseCharacter) // Parent Class

