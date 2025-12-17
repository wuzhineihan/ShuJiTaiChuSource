# BP_Breakable_02

**Asset Path**: `/Game/VRGamePlay/Projectile/Blueprint/Breakable/BP_Breakable_02.BP_Breakable_02`

---

## Metadata

- **Class**: BP_Breakable_02
- **ParentClass**: BP_Breakable_01_C

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
    * Call Parent: [ReceiveBeginPlay](#bp-breakable-01-receivebeginplay)()
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
    * Call Parent: [ReceiveTick](#bp-breakable-01-receivetick)(DeltaSeconds=Tick.DeltaSeconds)
        * [Path ends after call to function "ReceiveTick"]
```
---



---

### [Function] UserConstructionScript

**Trace Start: Construction Script** (FunctionEntry)

```blueprint
* Event Construction Script
    * Call Parent: [UserConstructionScript](#bp-breakable-01-userconstructionscript)()
        * [Path ends after call to function "UserConstructionScript"]
```
---



---

## References

- /Game/VRGamePlay/Projectile/Blueprint/Breakable/BP_Breakable_01.BP_Breakable_01_C (BP_Breakable_01_C) // Parent Class
- /Game/VRGamePlay/Projectile/Blueprint/Breakable/BP_Breakable_01.SKEL_BP_Breakable_01_C (SKEL_BP_Breakable_01_C) // EventGraph :: Parent: BeginPlay

