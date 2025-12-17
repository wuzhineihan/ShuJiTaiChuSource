# BP_ActorCanBeHit_NoiseMaker

**Asset Path**: `/Game/VRGamePlay/Projectile/Blueprint/BeHit/BP_ActorCanBeHit_NoiseMaker.BP_ActorCanBeHit_NoiseMaker`

---

## Metadata

- **Class**: BP_ActorCanBeHit_NoiseMaker
- **ParentClass**: Actor
- **Interfaces**:
  - BPI_TakeDamage_New_C

---

## Components

- SM_Tank : StaticMeshComponent
- Audio : AudioComponent (Parent: SM_Tank)

---

## Variables

(None)

---

## Functions

- UserConstructionScript -> void
- Event BeginPlay (Event) -> void
- Event ActorBeginOverlap (Event) -> void ( OtherActor: Actor )
- Event Tick (Event) -> void ( DeltaSeconds: real )
- Event Take Damage (Interface Implementation) (Event) -> void ( DamageInfo: Struct_DamageInfo )

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
    * Call Parent [ReceiveBeginPlay](#engine-receivebeginplay)()
        * Audio.SetWaveParameter((InName=Wave, InWave=SW_TankHit))
            * [Path ends]
```
---

**Trace Start: Event ActorBeginOverlap**

```blueprint
* Event ReceiveActorBeginOverlap Args: (OtherActor:Actor)
    * Call Parent [ReceiveActorBeginOverlap](#engine-receiveactorbeginoverlap)(OtherActor=ReceiveActorBeginOverlap.OtherActor)
        * [Path ends]
```
---

**Trace Start: Event Tick**

```blueprint
* Event Tick Args: (DeltaSeconds:float)
    * Call Parent [ReceiveTick](#engine-receivetick)(DeltaSeconds=Tick.DeltaSeconds)
        * [Path ends]
```
---

**Trace Start: Event Take Damage
From BPI Take Damage New**

```blueprint
* Event TakeDamage Args: (DamageInfo:Struct_DamageInfo)
    * VRTest:MyFunctionLibrary.FindNearestEnemy((Location=K2_GetActorLocation()))
        * MakeNoise((MaxRange=1500, NoiseInstigator=GetPlayerPawn(), NoiseLocation=K2_GetActorLocation())) (Target: Object)
            * Audio.Play(())
                * [Path ends]
```
---

**Trace Start: Print String** (CallFunction)

```blueprint
* PrintString((InString=NoiseMade)) (Target: KismetSystemLibrary)
    * [Path ends]
```
---

---

### [Function] UserConstructionScript

**Trace Start: Construction Script** (FunctionEntry)

```blueprint
* Event Construction Script
    * Call Parent [UserConstructionScript](#engine-userconstructionscript)()
        * [Path ends]
```
---



---

## References

- /Game/Audio/SoundWaves/SW_TankHit.SW_TankHit (SoundWave) // EventGraph :: Set Wave Parameter.InWave
- /Game/VRGamePlay/Damage/BPI_TakeDamage_New.BPI_TakeDamage_New_C (BPI_TakeDamage_New_C) // Implemented Interface
- /Script/VRTest.Default__MyFunctionLibrary (MyFunctionLibrary) // EventGraph :: Find Nearest Enemy.self
- /Script/VRTest.MyFunctionLibrary (MyFunctionLibrary) // EventGraph :: Find Nearest Enemy.self

