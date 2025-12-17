# Field

**Asset Path**: `/Game/VRGamePlay/Projectile/Blueprint/Field.Field`

---

## Metadata

- **Class**: Field
- **ParentClass**: FS_MasterField_C

---

## Components

- FieldSystemComponent : FieldSystemComponent

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
    * Call Parent: [ReceiveBeginPlay](#fs-masterfield-receivebeginplay)()
        * [Path ends after call to function "ReceiveBeginPlay"]
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
    * Call Parent: [ReceiveTick](#fs-masterfield-receivetick)(DeltaSeconds=Tick.DeltaSeconds)
        * [Path ends after call to function "ReceiveTick"]
```
---



---

### [Function] UserConstructionScript

**Trace Start: Construction Script** (FunctionEntry)

```blueprint
* Event Construction Script
    * Call Parent: [UserConstructionScript](#fs-masterfield-userconstructionscript)()
        * [Path ends after call to function "UserConstructionScript"]
```
---



---

## References

- /Engine/EditorResources/FieldNodes/FS_MasterField.FS_MasterField_C (FS_MasterField_C) // Parent Class
- /Engine/EditorResources/FieldNodes/FS_MasterField.SKEL_FS_MasterField_C (SKEL_FS_MasterField_C) // EventGraph :: Parent: BeginPlay

