# BP_GravityGloveDampComponent

**Asset Path**: `/Game/VRGamePlay/Player/VROps/GravityGloveDamping/BP_GravityGloveDampComponent.BP_GravityGloveDampComponent`

---

## Metadata

- **Class**: BP_GravityGloveDampComponent
- **ParentClass**: ActorComponent

---

## Components

(None)

---

## Variables

- DampCurve : CurveFloat (Public) // Damp Curve
- MotionController : BP_MyMotionController_C (Public) // Motion Controller
- StaticMeshComponent : StaticMeshComponent (Public) // Static Mesh Component

---

## Functions

- Event Begin Play (Event) -> void
- Event Tick (Event) -> void ( DeltaSeconds: real )

---

## Graph Inventory

- **Event**: 1

---

## Graph Logic

### [Event] EventGraph

**Trace Start: Event Begin Play**

```blueprint
* Event BeginPlay
    * Cast (RootComponent) To StaticMeshComponent
        |-- then:
        |   * Set StaticMeshComponent:StaticMeshComponent = Cast<StaticMeshComponent>(RootComponent)
        |       * [Path ends]
        L-- CastFailed:
            * PrintString((InString=ParentRootComponentNotStaitcMesh)) (Target: KismetSystemLibrary)
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

## References

- /Game/VRGamePlay/Player/VROps/BP_MyMotionController.BP_MyMotionController_C (BP_MyMotionController_C) // EventGraph :: Get MotionController.MotionController

