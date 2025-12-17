# BP_DragonImage

**Asset Path**: `/Game/VRGamePlay/Skill/SkillManger/StarImage/BP_DragonImage.BP_DragonImage`

---

## Metadata

- **Class**: BP_DragonImage
- **ParentClass**: Actor

---

## Components

- DefaultSceneRoot : SceneComponent
- Plane : StaticMeshComponent (Parent: DefaultSceneRoot)

---

## Variables

- MID : MaterialInstanceDynamic (Public) // MID

---

## Functions

- UserConstructionScript -> void
- SetOpacity -> void ( OpacityMulti: real )
- SetEmissive -> void ( EmissiveMulti: real )
- Event BeginPlay (Event) -> void
- Event ActorBeginOverlap (Event) -> void ( OtherActor: Actor )
- Event Tick (Event) -> void ( DeltaSeconds: real )

---

## Graph Inventory

- **Event**: 1
- **Function**: 3

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
    * Plane.CreateDynamicMaterialInstance((SourceMaterial=MT_StarImage))
        * Set MID:MaterialInstanceDynamic = Plane.CreateDynamicMaterialInstance(SourceMaterial=MT_StarImage)
            * [Path ends]
```
---



---

### [Function] SetOpacity

**Trace Start: Set Opacity** (FunctionEntry)

```blueprint
* Event Set Opacity
    * MID.SetScalarParameterValue((ParameterName=OpacityMulti, Value=ValueFrom(Set Opacity.OpacityMulti)))
        * [Path ends]
```
---



---

### [Function] SetEmissive

**Trace Start: Set Emissive** (FunctionEntry)

```blueprint
* Event Set Emissive
    * MID.SetScalarParameterValue((ParameterName=EmissiveMulti, Value=ValueFrom(Set Emissive.EmissiveMulti)))
        * [Path ends]
```
---



---

## References

- /Game/VRGamePlay/Skill/SkillManger/StarImage/MT_StarImage.MT_StarImage (Material) // UserConstructionScript :: Create Dynamic Material Instance.SourceMaterial

