# BP_BigTorch

**Asset Path**: `/Game/VRGamePlay/Scene/Torches/BP_BigTorch.BP_BigTorch`

---

## Metadata

- **Class**: BP_BigTorch
- **ParentClass**: Actor

---

## Components

- StaticMesh : StaticMeshComponent
- ParticleSystem : ParticleSystemComponent (Parent: StaticMesh)
- PointLight : PointLightComponent (Parent: StaticMesh)
- BPC_FireVolume : BPC_FireVolume_C (Parent: StaticMesh)

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

- /Game/VRGamePlay/Scene/Torches/BPC_FireVolume.BPC_FireVolume_C (BPC_FireVolume_C) // Component: BPC_FireVolume

