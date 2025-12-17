# UseGrab

**Asset Path**: `/Game/VRGamePlay/BeginnersGuide/TriggerBox/UseGrab.UseGrab`

---

## Metadata

- **Class**: UseGrab
- **ParentClass**: BP_BeginnersGuideTriggerBox_C

---

## Components

- CollisionComp : BoxComponent
- Sprite : BillboardComponent (Parent: CollisionComp)

---

## Variables

(None)

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

**Trace Start: Event BeginPlay**

```blueprint
* Event BeginPlay
    * Call Parent [ReceiveBeginPlay](#bp-beginnersguidetriggerbox-receivebeginplay)()
        * [Path ends]
```
---

**Trace Start: Event ActorBeginOverlap**

```blueprint
* Event ReceiveActorBeginOverlap Args: (OtherActor:Actor)
    * Call Parent: [ReceiveActorBeginOverlap](#bp-beginnersguidetriggerbox-receiveactorbeginoverlap)(OtherActor=ReceiveActorBeginOverlap.OtherActor)
        * [Path ends after call to function "ReceiveActorBeginOverlap"]
```
---

**Trace Start: Event Tick**

```blueprint
* Event Tick Args: (DeltaSeconds:float)
    * Call Parent [ReceiveTick](#bp-beginnersguidetriggerbox-receivetick)(DeltaSeconds=Tick.DeltaSeconds)
        * [Path ends]
```
---



---

### [Function] UserConstructionScript

**Trace Start: Construction Script** (FunctionEntry)

```blueprint
* Event Construction Script
    * Call Parent [UserConstructionScript](#bp-beginnersguidetriggerbox-userconstructionscript)()
        * [Path ends]
```
---



---

### [Function] ActualFunction

**Trace Start: Actual Function** (FunctionEntry)

```blueprint
* Event Actual Function
    * Call Parent: [ActualFunction](#bp-beginnersguidetriggerbox-actualfunction)()
        * VRPawn.Call Custom Event: [BP_VRPawn.UseGrabGuide](#bp-vrpawn-usegrabguide)()
            * [Path ends after call to custom event "BP_VRPawn.UseGrabGuide"]
```
---



---

## References

- /Game/VRGamePlay/BeginnersGuide/BP_BeginnersGuideTriggerBox.BP_BeginnersGuideTriggerBox_C (BP_BeginnersGuideTriggerBox_C) // ActualFunction :: Parent: Actual Function.self
- /Game/VRGamePlay/BeginnersGuide/BP_BeginnersGuideTriggerBox.SKEL_BP_BeginnersGuideTriggerBox_C (SKEL_BP_BeginnersGuideTriggerBox_C) // EventGraph :: Parent: BeginPlay
- /Game/VRGamePlay/Player/BP_VRPawn.BP_VRPawn_C (BP_VRPawn_C) // ActualFunction :: Get VRPawn.VRPawn
- /Game/VRGamePlay/Player/BP_VRPawn.SKEL_BP_VRPawn_C (SKEL_BP_VRPawn_C) // ActualFunction :: Use Grab Guide

