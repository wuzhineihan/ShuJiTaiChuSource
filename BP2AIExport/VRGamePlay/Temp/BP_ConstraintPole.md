# BP_ConstraintPole

**Asset Path**: `/Game/VRGamePlay/Temp/BP_ConstraintPole.BP_ConstraintPole`

---

## Metadata

- **Class**: BP_ConstraintPole
- **ParentClass**: Actor
- **Interfaces**:
  - BPI_TakeDamage_New_C

---

## Components

- StaticMesh : StaticMeshComponent
- PhysicsConstraintUp : PhysicsConstraintComponent (Parent: StaticMesh)
- PhysicsConstraintDown : PhysicsConstraintComponent (Parent: StaticMesh)

---

## Variables

(None)

---

## Functions

- UserConstructionScript -> void
- Event BeginPlay (Event) -> void
- Event ActorBeginOverlap (Event) -> void ( OtherActor: Actor )
- Event Tick (Event) -> void ( DeltaSeconds: real )
- Event Take Damage (Interface Implementation) (Event) -> void ( DamageInfo: Struct_DamageInfo )

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

**Trace Start: Event Take Damage
From BPI Take Damage New**

```blueprint
* Event TakeDamage Args: (DamageInfo:Struct_DamageInfo)
    * K2_DestroyActor(()) (Target: Actor)
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

- /Game/VRGamePlay/Damage/BPI_TakeDamage_New.BPI_TakeDamage_New_C (BPI_TakeDamage_New_C) // Implemented Interface

