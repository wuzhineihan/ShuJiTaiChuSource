# BP_ActorCanBeGrab_Child10

**Asset Path**: `/Game/VRGamePlay/Projectile/Blueprint/CanNotBreak/BP_ActorCanBeGrab_Child10.BP_ActorCanBeGrab_Child10`

---

## Metadata

- **Class**: BP_ActorCanBeGrab_Child10
- **ParentClass**: BP_ActorCanBeGrab_C

---

## Components

(None)

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
    * Call Parent [ReceiveBeginPlay](#bp-worlddynamicobject-receivebeginplay)()
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
    * Call Parent: [ReceiveTick](#bp-actorcanbegrab-receivetick)(DeltaSeconds=Tick.DeltaSeconds)
        * [Path ends after call to function "ReceiveTick"]
```
---



---

### [Function] UserConstructionScript

**Trace Start: Construction Script** (FunctionEntry)

```blueprint
* Event Construction Script
    * Call Parent: [UserConstructionScript](#bp-actorcanbegrab-userconstructionscript)()
        * [Path ends after call to function "UserConstructionScript"]
```
---



---

## References

- /Game/VRGamePlay/Projectile/Blueprint/BP_WorldDynamicObject.SKEL_BP_WorldDynamicObject_C (SKEL_BP_WorldDynamicObject_C) // EventGraph :: Parent: BeginPlay
- /Game/VRGamePlay/Projectile/Blueprint/CanNotBreak/BP_ActorCanBeGrab.BP_ActorCanBeGrab_C (BP_ActorCanBeGrab_C) // Parent Class
- /Game/VRGamePlay/Projectile/Blueprint/CanNotBreak/BP_ActorCanBeGrab.SKEL_BP_ActorCanBeGrab_C (SKEL_BP_ActorCanBeGrab_C) // EventGraph :: Parent: Tick

