# BP_CheckPoint

**Asset Path**: `/Game/VRGamePlay/SaveAndLoad/BP_CheckPoint.BP_CheckPoint`

---

## Metadata

- **Class**: BP_CheckPoint
- **ParentClass**: Actor

---

## Components

- DefaultSceneRoot : SceneComponent
- Box : BoxComponent (Parent: DefaultSceneRoot)

---

## Variables

(None)

---

## Functions

- UserConstructionScript -> void
- Event BeginPlay (Event) -> void
- Event ActorBeginOverlap (Event) -> void ( OtherActor: Actor )
- Event Tick (Event) -> void ( DeltaSeconds: real )
- On Component Begin Overlap (Box) (Event) -> void ( OverlappedComponent: PrimitiveComponent, OtherActor: Actor, OtherComp: PrimitiveComponent, OtherBodyIndex: int, bFromSweep: bool, SweepResult: HitResult )

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

**Trace Start: On Component Begin Overlap (Box)** (ComponentBoundEvent)

```blueprint
* Bound Event OnComponentBeginOverlap (Box on BP_CheckPoint)
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

