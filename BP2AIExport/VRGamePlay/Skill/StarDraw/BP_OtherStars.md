# BP_OtherStars

**Asset Path**: `/Game/VRGamePlay/Skill/StarDraw/BP_OtherStars.BP_OtherStars`

---

## Metadata

- **Class**: BP_OtherStars
- **ParentClass**: Actor

---

## Components

- SphereCollision : SphereComponent
- Sphere : StaticMeshComponent (Parent: SphereCollision)

---

## Variables

- DirectionFlag : int (Public) // Direction Flag

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

