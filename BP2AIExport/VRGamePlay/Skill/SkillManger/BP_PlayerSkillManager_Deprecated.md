# BP_PlayerSkillManager_Deprecated

**Asset Path**: `/Game/VRGamePlay/Skill/SkillManger/BP_PlayerSkillManager_Deprecated.BP_PlayerSkillManager_Deprecated`

---

## Metadata

- **Class**: BP_PlayerSkillManager_Deprecated
- **ParentClass**: Actor

---

## Components

- DefaultSceneRoot : SceneComponent

---

## Variables

- VRPawn : BP_VRPawn_C (Public) // VRPawn
- CanEagleEye : bool (Public) // Can Eagle Eye
- SkillMap : E_SiXiang (Public) // Skill Map

---

## Functions

- UserConstructionScript -> void
- ActivateSkill -> void ( SkillType: E_SiXiang )
- Event BeginPlay (Event) -> void
- Event ActorBeginOverlap (Event) -> void ( OtherActor: Actor )
- Event Tick (Event) -> void ( DeltaSeconds: real )
- ResumeEagleEye (Event) -> void
- EagleEye (Event) -> void

---

## Graph Inventory

- **Event**: 1
- **Function**: 2

---

## Graph Logic

### [Event] EventGraph

**Trace Start: EagleEye
Custom Event** (CustomEvent)

```blueprint
* Event EagleEye
    * If (CanEagleEye)
        * true:
            * Set CanEagleEye:bool = false
                * K2_SetTimerDelegate((Event=ResumeEagleEye.OutputDelegate, Time=10)) (Target: KismetSystemLibrary)
                    * Timeline: Timeline
                        * CallMaterialParameterCollectionFunction (Set Scalar Parameter Value)
                            * CallMaterialParameterCollectionFunction (Set Scalar Parameter Value)
                                * CallMaterialParameterCollectionFunction (Set Scalar Parameter Value)
                                    * [Path ends]
```
---

**Trace Start: ResumeEagleEye
Custom Event** (CustomEvent)

```blueprint
* Event ResumeEagleEye
    * Set CanEagleEye:bool = true
        * [Path ends]
```
---

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

### [Function] ActivateSkill

**Trace Start: Activate Skill** (FunctionEntry)

```blueprint
* Event Activate Skill
    * If (Map_Find(Key=ValueFrom(Activate Skill.SkillType), TargetMap=SkillMap))
        |-- true:
        |   * Switch on Enum E_Skill ((Map_Find(Key=ValueFrom(Activate Skill.SkillType), TargetMap=SkillMap)).Value)
        |       |-- EagleEye:
        |       |   * Call Custom Event: [EagleEye](#bp-playerskillmanager-deprecated-eagleeye)()
        |       |       * [Path ends after call to custom event "EagleEye"]
        |       L-- None:
        |           * PrintString((InString=No Equiped Skill)) (Target: KismetSystemLibrary)
        |               * [Path ends]
        L-- false:
            * PrintString((InString=Crazy Error!!!!)) (Target: KismetSystemLibrary)
                * [Path ends]
```
---



---

## References

- /Game/VRGamePlay/Player/BP_VRPawn.BP_VRPawn_C (BP_VRPawn_C) // Variable: VRPawn

