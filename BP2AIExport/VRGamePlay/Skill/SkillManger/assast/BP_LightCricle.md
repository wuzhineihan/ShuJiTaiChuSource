# BP_LightCricle

**Asset Path**: `/Game/VRGamePlay/Skill/SkillManger/assast/BP_LightCricle.BP_LightCricle`

---

## Metadata

- **Class**: BP_LightCricle
- **ParentClass**: Actor

---

## Components

- DefaultSceneRoot : SceneComponent
- NS_LightCricle : NiagaraComponent (Parent: DefaultSceneRoot)
- NS_LightCricle1 : NiagaraComponent (Parent: DefaultSceneRoot)
- NS_LightCricle2 : NiagaraComponent (Parent: DefaultSceneRoot)
- NS_LightCricle3 : NiagaraComponent (Parent: DefaultSceneRoot)
- NS_LightCricle4 : NiagaraComponent (Parent: DefaultSceneRoot)

---

## Variables

(None)

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
    * NS_LightCricle.SetVariableFloat((InValue=40, InVariableName=User.SpawnRate))
        * NS_LightCricle1.SetVariableFloat((InValue=80, InVariableName=User.SpawnRate))
            * NS_LightCricle2.SetVariableFloat((InValue=100, InVariableName=User.SpawnRate))
                * NS_LightCricle3.SetVariableFloat((InValue=50, InVariableName=User.SpawnRate))
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

## References

- /Script/Niagara.NiagaraComponent (NiagaraComponent) // EventGraph :: Set Niagara Variable (Float).self

