# BP_WorldDynamic_Grabbable

**Asset Path**: `/Game/VRGamePlay/Projectile/Blueprint/BP_WorldDynamic_Grabbable.BP_WorldDynamic_Grabbable`

---

## Metadata

- **Class**: BP_WorldDynamic_Grabbable
- **ParentClass**: BP_WorldDynamicObject_C

---

## Components

- BaseGrabComponent : BP_MyGrabComponent_C (Parent: BaseStaticMesh)

---

## Variables

(None)

---

## Functions

- UserConstructionScript -> void
- EnterStasis -> TimerHandle ( TimeToStasis: real )
- ToggleStasisGrabState -> void ( ActivateGrab: bool )
- IsInStasis -> bool
- Event BeginPlay (Event) -> void
- Event ActorBeginOverlap (Event) -> void ( OtherActor: Actor )
- Event Tick (Event) -> void ( DeltaSeconds: real )
- Event Exit Stasis (Event) -> void

---

## Graph Inventory

- **Event**: 1
- **Function**: 4

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
    * Call Parent [ReceiveActorBeginOverlap](#bp-worlddynamicobject-receiveactorbeginoverlap)(OtherActor=ReceiveActorBeginOverlap.OtherActor)
        * [Path ends]
```
---

**Trace Start: Event Tick**

```blueprint
* Event Tick Args: (DeltaSeconds:float)
    * Call Parent [ReceiveTick](#bp-worlddynamicobject-receivetick)(DeltaSeconds=Tick.DeltaSeconds)
        * [Path ends]
```
---

**Trace Start: Event Exit Stasis
From BPI Stasisable**

```blueprint
* Event ExitStasis
    * Call Interface: [ExitStasis](#bpi-stasisable-exitstasis)()
        * Call Function: [ToggleStasisGrabState](#bp-worlddynamic-grabbable-togglestasisgrabstate)(ActivateGrab=true)
            * [Path ends after call to function "ToggleStasisGrabState"]
```
---



---

### [Function] UserConstructionScript

**Trace Start: Construction Script** (FunctionEntry)

```blueprint
* Event Construction Script
    * Call Parent [UserConstructionScript](#bp-worlddynamicobject-userconstructionscript)()
        * [Path ends]
```
---



---

### [Function] EnterStasis

**Trace Start: Enter Stasis** (FunctionEntry)

```blueprint
* Event Enter Stasis
    * Call Interface: [EnterStasis](#bpi-stasisable-enterstasis)(TimeToStasis=ValueFrom(Enter Stasis.TimeToStasis))
        * Call Function: [ToggleStasisGrabState](#bp-worlddynamic-grabbable-togglestasisgrabstate)()
            * Return(TimerHandle=([EnterStasis](#bp-worlddynamicobject-enterstasis)(TimeToStasis=ValueFrom(Enter Stasis.TimeToStasis))).TimerHandle)
```
---



---

### [Function] ToggleStasisGrabState

**Trace Start: Toggle Stasis Grab State** (FunctionEntry)

```blueprint
* Event Toggle Stasis Grab State
    * Set CanGrab:bool = ValueFrom(Toggle Stasis Grab State.ActivateGrab) on BaseGrabComponent
        * Set GravityGlovesEnabled:bool = ValueFrom(Toggle Stasis Grab State.ActivateGrab) on BaseGrabComponent
            * [Path ends]
```
---



---

### [Function] IsInStasis

**Trace Start: Is in Stasis** (FunctionEntry)

```blueprint
* Event Is in Stasis
    * Call Interface: [IsInStasis](#bpi-stasisable-isinstasis)()
        * Return(IsInStasis=([IsInStasis](#bp-worlddynamicobject-isinstasis)()).IsInStasis)
```
---



---

## References

- /Game/VRGamePlay/Player/VROps/BP_MyGrabComponent.BP_MyGrabComponent_C (BP_MyGrabComponent_C) // ToggleStasisGrabState :: Get BaseGrabComponent.BaseGrabComponent
- /Game/VRGamePlay/Projectile/Blueprint/BP_WorldDynamicObject.BP_WorldDynamicObject_C (BP_WorldDynamicObject_C) // EventGraph :: Parent: Exit Stasis.self
- /Game/VRGamePlay/Projectile/Blueprint/BP_WorldDynamicObject.SKEL_BP_WorldDynamicObject_C (SKEL_BP_WorldDynamicObject_C) // EventGraph :: Parent: BeginPlay

