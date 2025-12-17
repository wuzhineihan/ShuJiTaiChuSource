# PhysicsCube

**Asset Path**: `/Game/VRGamePlay/Combat/TestContent/PhysicsCube.PhysicsCube`

---

## Metadata

- **Class**: PhysicsCube
- **ParentClass**: Actor

---

## Components

- DefaultSceneRoot : SceneComponent
- parent : StaticMeshComponent (Parent: DefaultSceneRoot)
- PhysicsControl : PhysicsControlComponent (Parent: DefaultSceneRoot)
- Sphere : StaticMeshComponent (Parent: DefaultSceneRoot)

---

## Variables

- lock : bool (Public) // Lock

---

## Functions

- UserConstructionScript -> void
- Event BeginPlay (Event) -> void
- Event ActorBeginOverlap (Event) -> void ( OtherActor: Actor )
- Event Tick (Event) -> void ( DeltaSeconds: real )
- Doit (Event) -> void

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
    * PhysicsControl.CreateControl((ControlData=Make<PhysicsControl:PhysicsControlData>(Linear Strength=10, Angular Strength=1, Custom Control Point=Vector(X=0, Y=0, Z=0))))
        * [Path ends]
```
---

**Trace Start: Doit
Custom Event** (CustomEvent)

```blueprint
* Event Doit
    * Set lock:bool = true
        * GetActorOfClass((ActorClass=BP_EnemyCharacter)) (Target: GameplayStatics)
            * PhysicsControl.CreateControl((ChildBoneName=pelvis, ChildMeshComponent=Mesh, ControlData=Make<PhysicsControl:PhysicsControlData>(Linear Strength=2, Angular Strength=1, Custom Control Point=Vector(X=0, Y=0, Z=0))))
                * [Path ends]
```
---

**Trace Start: Event Tick**

```blueprint
* Event Tick Args: (DeltaSeconds:float)
    * PhysicsControl.SetControlTarget((Control Target Target Angular Velocity=Vector(X=0, Y=0, Z=0), Control Target Target Orientation=parent.K2_GetComponentRotation(), Control Target Target Position=parent.K2_GetComponentLocation(), Control Target Target Velocity=Vector(X=0, Y=0, Z=0), Name=PhysicsControl.CreateControl(ControlData=Make<PhysicsControl:PhysicsControlData>(Linear Strength=10, Angular Strength=1, Custom Control Point=Vector(X=0, Y=0, Z=0)))))
        * If (lock)
            * true:
                * PhysicsControl.SetControlTarget((Control Target Target Angular Velocity=Vector(X=0, Y=0, Z=0), Control Target Target Orientation=parent.K2_GetComponentRotation(), Control Target Target Position=parent.K2_GetComponentLocation(), Control Target Target Velocity=Vector(X=0, Y=0, Z=0), Name=PhysicsControl.CreateControl(ChildBoneName=pelvis, ChildMeshComponent=Mesh, ControlData=Make<PhysicsControl:PhysicsControlData>(Linear Strength=2, Angular Strength=1, Custom Control Point=Vector(X=0, Y=0, Z=0)))))
                    * [Path ends]
```
---

**Trace Start: Event ActorBeginOverlap**

```blueprint
* Event ReceiveActorBeginOverlap Args: (OtherActor:Actor)
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

## References

- /Game/enemy_ai_test/ai_enmey/BP_EnemyCharacter.BP_EnemyCharacter_C (BP_EnemyCharacter_C) // EventGraph :: Get Actor Of Class.ActorClass
- /Script/PhysicsControl.PhysicsControlComponent (PhysicsControlComponent) // EventGraph :: Get PhysicsControl.PhysicsControl

