# BP_VRPlayerState

**Asset Path**: `/Game/VRGamePlay/Player/BP_VRPlayerState.BP_VRPlayerState`

---

## Metadata

- **Class**: BP_VRPlayerState
- **ParentClass**: PlayerState

---

## Components

- DefaultSceneRoot : SceneComponent

---

## Variables

- SkillLearnedMap : E_Skill (Public) // Skill Learned Map
- CanStarDraw : bool (Public) // Can Star Draw
- ArrowNumber : int (Public) // Arrow Number
- BowInBackPack : bool (Public) // Bow in Back Pack
- Magic : int (Public) // Magic

---

## Functions

- UserConstructionScript -> void
- LearnSkill -> void ( SkillToLearn: E_Skill )
- Event BeginPlay (Event) -> void
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

### [Function] LearnSkill

**Trace Start: Learn Skill** (FunctionEntry)

```blueprint
* Event Learn Skill
    * Map_Add((Key=ValueFrom(Learn Skill.SkillToLearn), TargetMap=SkillLearnedMap, Value=true)) (Target: BlueprintMapLibrary)
        * [Path ends]
```
---



---

