# BP_FX

**Asset Path**: `/Game/VRGamePlay/Skill/SkillManger/assast/BP_FX.BP_FX`

---

## Metadata

- **Class**: BP_FX
- **ParentClass**: Actor

---

## Components

- DefaultSceneRoot : SceneComponent
- NS_ReshapeStar : NiagaraComponent (Parent: DefaultSceneRoot)
- Plane : StaticMeshComponent (Parent: DefaultSceneRoot)

---

## Variables

- DetectDistant : int (Public) // Detect Distant
- NewVar : bool (Public) // New Var

---

## Functions

- UserConstructionScript -> void
- DetectSight -> void
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

### [Function] DetectSight

**Trace Start: Detect Sight** (FunctionEntry)

```blueprint
* Event Detect Sight
    * LineTraceSingle((End=((GetPlayerController().GetActorEyesViewPoint()).OutLocation + (GetForwardVector(InRot=(GetPlayerController().GetActorEyesViewPoint()).OutRotation) * DetectDistant)), Start=(GetPlayerController().GetActorEyesViewPoint()).OutLocation)) (Target: KismetSystemLibrary)
        * [Path ends]
```
---



---

## References

- /Script/Niagara.NiagaraComponent (NiagaraComponent) // Component: NS_ReshapeStar
- /Script/PhysicsCore.PhysicalMaterial (PhysicalMaterial) // DetectSight :: Break Hit Result.PhysMat

