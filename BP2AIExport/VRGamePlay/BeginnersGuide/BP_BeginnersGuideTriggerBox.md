# BP_BeginnersGuideTriggerBox

**Asset Path**: `/Game/VRGamePlay/BeginnersGuide/BP_BeginnersGuideTriggerBox.BP_BeginnersGuideTriggerBox`

---

## Metadata

- **Class**: BP_BeginnersGuideTriggerBox
- **ParentClass**: TriggerBox

---

## Components

- CollisionComp : BoxComponent
- Sprite : BillboardComponent (Parent: CollisionComp)

---

## Variables

- VRPawn : BP_VRPawn_C (Public) // VRPawn

---

## Functions

- UserConstructionScript -> void
- ActualFunction -> void
- Event BeginPlay (Event) -> void
- Event ActorBeginOverlap (Event) -> void ( OtherActor: Actor )
- Event Tick (Event) -> void ( DeltaSeconds: real )

---

## Graph Inventory

- **Event**: 1
- **Function**: 2

---

## Graph Logic

### [Event] EventGraph

**Trace Start: Event ActorBeginOverlap**

```blueprint
* Event ReceiveActorBeginOverlap Args: (OtherActor:Actor)
    * Cast (ReceiveActorBeginOverlap.OtherActor) To BP_VRPawn
        |-- then:
        |   * Set VRPawn:BP_VRPawn = Cast<BP_VRPawn>(ReceiveActorBeginOverlap.OtherActor)
        |       * If (ReceiveActorBeginOverlap.OtherActor.ActorHasTag(Tag=player))
        |           * true:
        |               * Call Function: [ActualFunction](#bp-beginnersguidetriggerbox-actualfunction)()
        |                   * K2_DestroyActor(()) (Target: Actor)
        |                       * [Path ends]
        L-- CastFailed:
            * PrintString((InString=CastFailed)) (Target: KismetSystemLibrary)
                * [Path ends]
```
---

**Trace Start: Event Tick**

```blueprint
* Event Tick Args: (DeltaSeconds:float)
    * [Path ends]
```
---

**Trace Start: Event BeginPlay**

```blueprint
* Event BeginPlay
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

### [Function] ActualFunction

**Trace Start: Actual Function** (FunctionEntry)

```blueprint
* Event Actual Function
    * [Path ends]
```
---

---

## References

- /Game/VRGamePlay/Player/BP_VRPawn.BP_VRPawn_C (BP_VRPawn_C) // EventGraph :: Cast To BP_VRPawn.AsBP VRPawn

