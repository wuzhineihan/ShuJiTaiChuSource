# BP_FingerPoint

**Asset Path**: `/Game/VRGamePlay/Skill/StarDraw/BP_FingerPoint.BP_FingerPoint`

---

## Metadata

- **Class**: BP_FingerPoint
- **ParentClass**: Actor

---

## Components

- SphereCollision : SphereComponent
- Sphere : StaticMeshComponent (Parent: SphereCollision)

---

## Variables

- DrawManager : BP_DrawManager_C (Public) // Draw Manager
- OtherStar : Actor (Public) // Other Star
- HandMesh : SkeletalMeshComponent (Public) // Hand Mesh
- LineTraceLength : real (Public) // Line Trace Length

---

## Functions

- UserConstructionScript -> void
- Event ActorBeginOverlap (Event) -> void ( OtherActor: Actor )
- On Component Begin Overlap (SphereCollision) (Event) -> void ( OverlappedComponent: PrimitiveComponent, OtherActor: Actor, OtherComp: PrimitiveComponent, OtherBodyIndex: int, bFromSweep: bool, SweepResult: HitResult )
- Event Tick (Event) -> void ( DeltaSeconds: real )

---

## Graph Inventory

- **Event**: 1
- **Function**: 1

---

## Graph Logic

### [Event] EventGraph

**Trace Start: Event ActorBeginOverlap**

```blueprint
* Event ReceiveActorBeginOverlap Args: (OtherActor:Actor)
    * [Path ends]
```
---

**Trace Start: On Component Begin Overlap (SphereCollision)** (ComponentBoundEvent)

```blueprint
* Bound Event OnComponentBeginOverlap (SphereCollision on BP_FingerPoint)
    * If (On Component Begin Overlap (SphereCollision on Owner).OtherActor.ActorHasTag(Tag=OtherStars))
        * true:
            * Cast (On Component Begin Overlap (SphereCollision on Owner).OtherActor) To BP_OtherStars
                |-- then:
                |   * DrawManager.Call Function: [BP_DrawManager.TouchOtherStars](#bp-drawmanager-touchotherstars)(LocalOtherStar=Cast<BP_OtherStars>(On Component Begin Overlap (SphereCollision on Owner).OtherActor))
                |       * [Path ends after call to function "BP_DrawManager.TouchOtherStars"]
                L-- CastFailed:
                    * PrintString((InString=Cast Failed)) (Target: KismetSystemLibrary)
                        * [Path ends]
```
---

**Trace Start: Event Tick**

```blueprint
* Event Tick Args: (DeltaSeconds:float)
    * SphereTraceSingleForObjects((End=SphereCollision.K2_GetComponentLocation(), ObjectTypes=[EObjectTypeQuery::WorldDynamic], Radius=5, Start=DrawManager.MotionController.K2_GetComponentLocation())) (Target: KismetSystemLibrary)
        * If (SphereTraceSingleForObjects(End=SphereCollision.K2_GetComponentLocation(), ObjectTypes=[EObjectTypeQuery::WorldDynamic], Radius=5, Start=DrawManager.MotionController.K2_GetComponentLocation(), TraceColor=LinearColor(R=1, G=0, B=0, A=1), TraceHitColor=LinearColor(R=0, G=1, B=0, A=1)))
            * true:
                * If ((SphereTraceSingleForObjects(End=SphereCollision.K2_GetComponentLocation(), ObjectTypes=[EObjectTypeQuery::WorldDynamic], Radius=5, Start=DrawManager.MotionController.K2_GetComponentLocation(), TraceColor=LinearColor(R=1, G=0, B=0, A=1), TraceHitColor=LinearColor(R=0, G=1, B=0, A=1))).OutHit_HitActor.ActorHasTag(Tag=OtherStars))
                    * true:
                        * Cast ((SphereTraceSingleForObjects(End=SphereCollision.K2_GetComponentLocation(), ObjectTypes=[EObjectTypeQuery::WorldDynamic], Radius=5, Start=DrawManager.MotionController.K2_GetComponentLocation(), TraceColor=LinearColor(R=1, G=0, B=0, A=1), TraceHitColor=LinearColor(R=0, G=1, B=0, A=1))).OutHit_HitActor) To BP_OtherStars
                            |-- then:
                            |   * DrawManager.Call Function: [BP_DrawManager.TouchOtherStars](#bp-drawmanager-touchotherstars)(LocalOtherStar=Cast<BP_OtherStars>((SphereTraceSingleForObjects(End=SphereCollision.K2_GetComponentLocation(), ObjectTypes=[EObjectTypeQuery::WorldDynamic], Radius=5, Start=DrawManager.MotionController.K2_GetComponentLocation(), TraceColor=LinearColor(R=1, G=0, B=0, A=1), TraceHitColor=LinearColor(R=0, G=1, B=0, A=1))).OutHit_HitActor))
                            |       * [Path ends after call to function "BP_DrawManager.TouchOtherStars"]
                            L-- CastFailed:
                                * PrintString((InString=Cast Failed)) (Target: KismetSystemLibrary)
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

- /Game/VRGamePlay/Player/VROps/BP_MyMotionController.BP_MyMotionController_C (BP_MyMotionController_C) // EventGraph :: Get MotionController.MotionController
- /Game/VRGamePlay/Skill/StarDraw/BP_DrawManager.BP_DrawManager_C (BP_DrawManager_C) // EventGraph :: Get DrawManager.DrawManager
- /Game/VRGamePlay/Skill/StarDraw/BP_DrawManager.SKEL_BP_DrawManager_C (SKEL_BP_DrawManager_C) // EventGraph :: Touch Other Stars
- /Game/VRGamePlay/Skill/StarDraw/BP_OtherStars.BP_OtherStars_C (BP_OtherStars_C) // EventGraph :: Touch Other Stars.LocalOtherStar
- /Script/PhysicsCore.PhysicalMaterial (PhysicalMaterial) // EventGraph :: Sphere Trace For Objects.OutHit_PhysMat

