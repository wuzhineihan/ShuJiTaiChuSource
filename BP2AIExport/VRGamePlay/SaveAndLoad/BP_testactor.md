# BP_testactor

**Asset Path**: `/Game/VRGamePlay/SaveAndLoad/BP_testactor.BP_testactor`

---

## Metadata

- **Class**: BP_testactor
- **ParentClass**: Actor
- **Interfaces**:
  - BPI_SaveableActor_C

---

## Components

- DefaultSceneRoot : SceneComponent
- StaticMesh : StaticMeshComponent (Parent: DefaultSceneRoot)

---

## Variables

- GUID : Guid (Public) // GUID
- IfCollected : bool (Public) // If Collected

---

## Functions

- UserConstructionScript -> void
- SaveInfo (Interface Implementation) -> Guid, Transform, Map<E_Skill, bool>, int, bool
- LoadInfo (Interface Implementation) -> void ( L_Location: Transform, L_Skill: Map<E_Skill, bool>, L_Magic: int, L_statue: bool )
- GetGuid (Interface Implementation) -> Guid
- Event New GUID (Interface Implementation) (Event) -> void

---

## Graph Inventory

- **Event**: 1
- **Function**: 1
- **Interface**: 3

---

## Graph Logic

### [Event] EventGraph

**Trace Start: Event New GUID
From BPI Saveable Actor**

```blueprint
* Event NewGUID
    * KismetGuidLibrary.Invalidate_Guid((InGuid=GUID))
        * Set GUID:CoreUObject:Guid = KismetGuidLibrary.NewGuid()
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

### [Interface] SaveInfo

**Trace Start: Save Info** (FunctionEntry)

```blueprint
* Event Save Info
    * Return(GUID=GUID, Location=GetTransform(), Statue=IfCollected)
```
---



---

### [Interface] LoadInfo

**Trace Start: Load Info** (FunctionEntry)

```blueprint
* Event Load Info
    * PrintString((InString=get information)) (Target: KismetSystemLibrary)
        * K2_SetActorTransform((NewTransform=ValueFrom(Load Info.L_Location))) (Target: Actor)
            * Return()
```
---



---

### [Interface] GetGuid

**Trace Start: Get Guid** (FunctionEntry)

```blueprint
* Event Get Guid
    * Return(TargetGUID=GUID)
```
---



---

## References

- /Game/VRGamePlay/SaveAndLoad/BPI_SaveableActor.BPI_SaveableActor_C (BPI_SaveableActor_C) // Implemented Interface

