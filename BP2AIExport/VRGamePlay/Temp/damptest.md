# damptest

**Asset Path**: `/Game/VRGamePlay/Temp/damptest.damptest`

---

## Metadata

- **Class**: damptest
- **ParentClass**: Actor

---

## Components

- DefaultSceneRoot : SceneComponent
- StaticMesh : StaticMeshComponent (Parent: DefaultSceneRoot)

---

## Variables

- New Track 0 : real (Public) // New Track 0

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
    * Timeline: Timeline
        * StaticMesh.SetLinearDamping((InDamping=Timeline.NewTrack_0))
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

