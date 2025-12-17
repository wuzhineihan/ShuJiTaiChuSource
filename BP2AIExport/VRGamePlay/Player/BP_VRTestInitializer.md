# BP_VRTestInitializer

**Asset Path**: `/Game/VRGamePlay/Player/BP_VRTestInitializer.BP_VRTestInitializer`

---

## Metadata

- **Class**: BP_VRTestInitializer
- **ParentClass**: Actor

---

## Components

- DefaultSceneRoot : SceneComponent

---

## Variables

- VRPawn : BP_VRPawn_C (Public) // VRPawn
- As BP VRPlayer State : BP_VRPlayerState_C (Public) // As BP VRPlayer State
- Initialize : bool (Public) // Initialize

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
    * If (Initialize)
        * true:
            * GetActorOfClass((ActorClass=BP_StartMenu)) (Target: GameplayStatics)
                * GetActorOfClass(ActorClass=BP_StartMenu).Call Custom Event: [BP_StartMenu.CloseMenu](#bp-startmenu-closemenu)() (Target: GameplayStatics)
                    * Cast (GetPlayerPawn()) To BP_VRPawn
                        * Set VRPawn:BP_VRPawn = Cast<BP_VRPawn>(GetPlayerPawn())
                            * Cast (GetPlayerState()) To BP_VRPlayerState
                                * Set As BP VRPlayer State:BP_VRPlayerState = Cast<BP_VRPlayerState>(GetPlayerState())
                                    * Set CanStarDraw:bool = true on (Cast<BP_VRPlayerState>(GetPlayerState()))
                                        * Cast<BP_VRPlayerState>(GetPlayerState()).Call Function: [BP_VRPlayerState.LearnSkill](#bp-vrplayerstate-learnskill)()
                                            * Set CanMove:bool = true on VRPawn
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

- /Game/ui/Menu/BP_StartMenu.BP_StartMenu_C (BP_StartMenu_C) // EventGraph :: Get Actor Of Class.ActorClass
- /Game/ui/Menu/BP_StartMenu.SKEL_BP_StartMenu_C (SKEL_BP_StartMenu_C) // EventGraph :: Close Menu
- /Game/VRGamePlay/Player/BP_VRPawn.BP_VRPawn_C (BP_VRPawn_C) // EventGraph :: Cast To BP_VRPawn.AsBP VRPawn
- /Game/VRGamePlay/Player/BP_VRPlayerState.BP_VRPlayerState_C (BP_VRPlayerState_C) // EventGraph :: Cast To BP_VRPlayerState.AsBP VRPlayer State
- /Game/VRGamePlay/Player/BP_VRPlayerState.SKEL_BP_VRPlayerState_C (SKEL_BP_VRPlayerState_C) // EventGraph :: Learn Skill

