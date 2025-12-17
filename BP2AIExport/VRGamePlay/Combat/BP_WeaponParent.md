# BP_WeaponParent

**Asset Path**: `/Game/VRGamePlay/Combat/BP_WeaponParent.BP_WeaponParent`

---

## Metadata

- **Class**: BP_WeaponParent
- **ParentClass**: BP_WorldDynamic_Grabbable_C
- **Interfaces**:
  - BPI_WeaponInfo_C

---

## Components

(None)

---

## Variables

- WeaponType : E_WeaponType (Public) // Weapon Type

---

## Functions

- UserConstructionScript -> void
- GetWeaponType (Interface Implementation) -> E_WeaponType
- Event BeginPlay (Event) -> void
- Event ActorBeginOverlap (Event) -> void ( OtherActor: Actor )
- Event Tick (Event) -> void ( DeltaSeconds: real )

---

## Graph Inventory

- **Event**: 1
- **Function**: 1
- **Interface**: 1

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

### [Interface] GetWeaponType

**Trace Start: Get Weapon Type** (FunctionEntry)

```blueprint
* Event Get Weapon Type
    * Return(WeaponType=WeaponType)
```
---



---

## References

- /Game/VRGamePlay/Combat/BPI_WeaponInfo.BPI_WeaponInfo_C (BPI_WeaponInfo_C) // Implemented Interface
- /Game/VRGamePlay/Projectile/Blueprint/BP_WorldDynamic_Grabbable.BP_WorldDynamic_Grabbable_C (BP_WorldDynamic_Grabbable_C) // Parent Class

