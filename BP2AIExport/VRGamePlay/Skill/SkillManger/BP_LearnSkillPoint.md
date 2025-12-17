# BP_LearnSkillPoint

**Asset Path**: `/Game/VRGamePlay/Skill/SkillManger/BP_LearnSkillPoint.BP_LearnSkillPoint`

---

## Metadata

- **Class**: BP_LearnSkillPoint
- **ParentClass**: Actor
- **Interfaces**:
  - BPI_CustomGrabAndRelease_C

---

## Components

- Scene : SceneComponent
- Sphere : SphereComponent (Parent: Scene)
- BP_MyGrabComponent : BP_MyGrabComponent_C (Parent: Sphere)
- ShatterAudio : AudioComponent (Parent: Sphere)
- FloatingAudio : AudioComponent (Parent: Sphere)
- FloatingVFX : NiagaraComponent (Parent: Sphere)
- BurstVFX : NiagaraComponent (Parent: FloatingVFX)

---

## Variables

- SkillToLearn : E_Skill (Public) // Skill to Learn
- SkillPointGrabbed : mcdelegate (Public) // Skill Point Grabbed

---

## Functions

- UserConstructionScript -> void
- BackpackToggle (Interface Implementation) -> bool ( Activate: bool )
- OnGrab (Interface Implementation) -> bool ( LocalMotionController: BP_MyMotionController_C, BoneName: name )
- Event BeginPlay (Event) -> void
- Event ActorBeginOverlap (Event) -> void ( OtherActor: Actor )
- Event Tick (Event) -> void ( DeltaSeconds: real )
- Event On Release (Interface Implementation) (Event) -> void ( LocalMotionController: BP_MyMotionController_C )
- WaitAndDestroySelf (Event) -> void

---

## Graph Inventory

- **Event**: 1
- **Function**: 1
- **Interface**: 2
- **Delegate**: 1

---

## Graph Logic

### [Event] EventGraph

**Trace Start: Event BeginPlay**

```blueprint
* Event BeginPlay
    * FloatingAudio.SetSound((NewSound=SW_StarFloating))
        * FloatingAudio.Play(())
            * Timeline: Timeline
                * FloatingAudio.SetVolumeMultiplier((NewVolumeMultiplier=Timeline.VolumeGoUp))
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

**Trace Start: Event On Release
From BPI Custom Grab and Release**

```blueprint
* Event OnRelease Args: (LocalMotionController:BP_MyMotionController)
    * [Path ends]
```
---

**Trace Start: WaitAndDestroySelf
Custom Event** (CustomEvent)

```blueprint
* Event WaitAndDestroySelf
    * Delay((Duration=10)) [(Latent)] (Target: KismetSystemLibrary)
        * K2_DestroyActor(()) (Target: Actor)
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

### [Interface] BackpackToggle

**Trace Start: Backpack Toggle** (FunctionEntry)

```blueprint
* Event Backpack Toggle
    * Return()
```
---



---

### [Interface] OnGrab

**Trace Start: On Grab** (FunctionEntry)

```blueprint
* Event On Grab
    * Call Delegate SkillPointGrabbed()
        * Cast (GetPlayerState()) To BP_VRPlayerState
            |-- then:
            |   * Cast<BP_VRPlayerState>(GetPlayerState()).Call Function: [BP_VRPlayerState.LearnSkill](#bp-vrplayerstate-learnskill)(SkillToLearn=SkillToLearn)
            |       * ShatterAudio.SetSound((NewSound=SW_StarShatter))
            |           * ShatterAudio.Play((StartTime=0.2))
            |               * FloatingAudio.Stop(())
            |                   * BurstVFX.Activate(())
            |                       * FloatingVFX.SetVisibility(())
            |                           * Call Custom Event: [WaitAndDestroySelf](#bp-learnskillpoint-waitanddestroyself)()
            |                               * Return()
            L-- CastFailed:
                * PrintString((InString=Wrong Player State)) (Target: KismetSystemLibrary)
                    * [Path ends]
```
---



---

### [Delegate] SkillPointGrabbed


---

## References

- /Game/Audio/SoundWaves/SW_StarFloating.SW_StarFloating (SoundWave) // EventGraph :: Set Sound.NewSound
- /Game/Audio/SoundWaves/SW_StarShatter.SW_StarShatter (SoundWave) // OnGrab :: Set Sound.NewSound
- /Game/VRGamePlay/Player/BP_VRPlayerState.BP_VRPlayerState_C (BP_VRPlayerState_C) // OnGrab :: Cast To BP_VRPlayerState.AsBP VRPlayer State
- /Game/VRGamePlay/Player/BP_VRPlayerState.SKEL_BP_VRPlayerState_C (SKEL_BP_VRPlayerState_C) // OnGrab :: Learn Skill
- /Game/VRGamePlay/Player/VROps/BP_MyGrabComponent.BP_MyGrabComponent_C (BP_MyGrabComponent_C) // Component: BP_MyGrabComponent
- /Game/VRGamePlay/Player/VROps/BP_MyMotionController.BP_MyMotionController_C (BP_MyMotionController_C) // EventGraph :: Event On Release.LocalMotionController
- /Game/VRGamePlay/Player/VROps/BPI_CustomGrabAndRelease.BPI_CustomGrabAndRelease_C (BPI_CustomGrabAndRelease_C) // Implemented Interface
- /Script/Niagara.NiagaraComponent (NiagaraComponent) // OnGrab :: Get BurstVFX.BurstVFX

