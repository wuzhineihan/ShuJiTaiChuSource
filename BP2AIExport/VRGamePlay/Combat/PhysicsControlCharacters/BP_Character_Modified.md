# BP_Character_Modified

**Asset Path**: `/Game/VRGamePlay/Combat/PhysicsControlCharacters/BP_Character_Modified.BP_Character_Modified`

---

## Metadata

- **Class**: BP_Character_Modified
- **ParentClass**: Enemy_Base
- **Interfaces**:
  - BPI_CanDrag_C

---

## Components

- CollisionCylinder : CapsuleComponent
- Arrow : ArrowComponent (Parent: CollisionCylinder)
- CharMoveComp : CharacterMovementComponent
- CharacterMesh0 : SkeletalMeshComponent (Parent: CollisionCylinder)
- PhysicsControl : PhysicsControlComponent (Parent: CollisionCylinder)
- PhysicsConstraint : PhysicsConstraintComponent (Parent: CharacterMesh0)
- EnemyStateWidget : WidgetComponent (Parent: CharacterMesh0)

---

## Variables

- Limb Setup Data : PhysicsControlLimbSetupData (Public) // Limb Setup Data
- WorldSpaceControlData : PhysicsControlData (Public) // World Space Control Data
- ParentSpaceControlData : PhysicsControlData (Public) // Parent Space Control Data
- Body Modifier Data : PhysicsControlModifierData (Public) // Body Modifier Data
- Physics Movement Type : EPhysicsMovementType (Public) // Physics Movement Type
- Gravity Multiplier : real (Public) // Gravity Multiplier
- ConstraintProfile : name (Public) // Constraint Profile
- WeaponSocketName : name (Public) // Weapon Socket Name

---

## Functions

- UserConstructionScript -> void
- CreateControls -> void
- CanDrag? (Interface Implementation) -> bool
- Event BeginPlay (Event) -> void
- UseWorldSpaceControls (Event) -> void
- UseParentSpaceControls (Event) -> void

---

## Graph Inventory

- **Event**: 1
- **Function**: 2
- **Interface**: 1

---

## Graph Logic

### [Event] EventGraph

**Trace Start: Event BeginPlay**

```blueprint
* Event BeginPlay
    * Call Function: [CreateControls](#bp-character-modified-createcontrols)()
        * Call Custom Event: [UseWorldSpaceControls](#bp-character-modified-useworldspacecontrols)()
            * [Path ends after call to custom event "UseWorldSpaceControls"]
```
---

**Trace Start: UseWorldSpaceControls
Custom Event** (CustomEvent)

```blueprint
* Event UseWorldSpaceControls
    * PhysicsControl.SetControlsInSetEnabled((Set=WorldSpace))
        * PhysicsControl.SetControlsInSetEnabled((Set=ParentSpace))
            * PhysicsControl.SetBodyModifiersInSetGravityMultiplier((Set=All))
                * PhysicsControl.SetBodyModifiersInSetMovementType((Set=Feet))
                    * PhysicsControl.SetBodyModifiersInSetMovementType((Set=LowerBody))
                        * [Path ends]
```
---

**Trace Start: UseParentSpaceControls
Custom Event** (CustomEvent)

```blueprint
* Event UseParentSpaceControls
    * PhysicsControl.SetControlsInSetEnabled((Set=WorldSpace))
        * PhysicsControl.SetControlsInSetEnabled((Set=ParentSpace))
            * PhysicsControl.SetBodyModifiersInSetGravityMultiplier((Set=All))
                * PhysicsControl.SetBodyModifiersInSetMovementType((Set=Feet))
                    * PhysicsControl.SetBodyModifiersInSetMovementType((Set=LowerBody))
                        * [Path ends]
```
---



---

### [Function] UserConstructionScript

**Trace Start: Construction Script** (FunctionEntry)

```blueprint
* Event Construction Script
    * PhysicsConstraint.K2_AttachToComponent((Parent=Mesh, SocketName=WeaponSocketName))
        * [Path ends]
```
---



---

### [Function] CreateControls

**Trace Start: Create Controls** (FunctionEntry)

```blueprint
* Event Create Controls
    * Sequence
        |-- then_0:
        |   * Mesh.SetConstraintProfileForAll((ProfileName=ConstraintProfile))
        |       * [Path ends]
        L-- then_1:
            * PhysicsControl.CreateControlsAndBodyModifiersFromLimbBones((BodyModifierData=Body Modifier Data(PhysicsControlModifierData), LimbSetupData=Limb Setup Data(PhysicsControlLimbSetupData), ParentSpaceControlData=ParentSpaceControlData(PhysicsControlData), SkeletalMeshComponent=Mesh, WorldSpaceControlData=WorldSpaceControlData(PhysicsControlData)))
                * Sequence
                    |-- then_0:
                    |   * PhysicsControl.AddBodyModifierToSet((BodyModifier=foot_l, Set=Feet))
                    |       * PhysicsControl.AddBodyModifierToSet((BodyModifier=foot_r, Set=Feet))
                    |           * PhysicsControl.AddBodyModifierToSet((BodyModifier=hand_l, Set=Hands))
                    |               * PhysicsControl.AddBodyModifierToSet((BodyModifier=hand_r, Set=Hands))
                    |                   * PhysicsControl.AddBodyModifiersToSet((BodyModifiers=PhysicsControl.GetBodyModifierNamesInSet(Set=LegLeft), Set=LowerBody))
                    |                       * PhysicsControl.AddBodyModifiersToSet((BodyModifiers=PhysicsControl.GetBodyModifierNamesInSet(Set=LegRight), Set=LowerBody))
                    |                           * PhysicsControl.AddBodyModifierToSet((BodyModifier=pelvis, Set=LowerBody))
                    |                               * [Path ends]
                    |-- then_1:
                    |   * PhysicsControl.AddControlsToSet((Controls=PhysicsControl.GetControlNamesInSet(Set=ParentSpace_ArmLeft), Set=ParentSpace_Arms))
                    |       * PhysicsControl.AddControlsToSet((Controls=PhysicsControl.GetControlNamesInSet(Set=ParentSpace_ArmRight), Set=ParentSpace_Arms))
                    |           * [Path ends]
                    L-- then_2:
                        * PhysicsControl.AddControlsToSet((Controls=PhysicsControl.GetControlNamesInSet(Set=WorldSpace_Spine), Set=WorldSpace_UpperBody))
                            * PhysicsControl.AddControlsToSet((Controls=PhysicsControl.GetControlNamesInSet(Set=WorldSpace_Head), Set=WorldSpace_UpperBody))
                                * PhysicsControl.AddControlsToSet((Controls=PhysicsControl.GetControlNamesInSet(Set=WorldSpace_ArmLeft), Set=WorldSpace_UpperBody))
                                    * PhysicsControl.AddControlsToSet((Controls=PhysicsControl.GetControlNamesInSet(Set=WorldSpace_ArmRight), Set=WorldSpace_UpperBody))
                                        * [Path ends]
```
---



---

### [Interface] CanDrag?

**Trace Start: Can Drag?** (FunctionEntry)

```blueprint
* Event Can Drag?
    * Return(CanDrag?=true)
```
---



---

## References

- /Game/VRGamePlay/Combat/PhysicsControlCharacters/BPI_CanDrag.BPI_CanDrag_C (BPI_CanDrag_C) // Implemented Interface
- /Script/PhysicsControl.PhysicsControlComponent (PhysicsControlComponent) // EventGraph :: Get PhysicsControl.PhysicsControl
- /Script/UMG.WidgetComponent (WidgetComponent) // Component: EnemyStateWidget
- /Script/VRTest.Enemy_Base (Enemy_Base) // Parent Class

