# BP_VRFunctionLibrary

**Asset Path**: `/Game/VRGamePlay/BP_VRFunctionLibrary.BP_VRFunctionLibrary`

---

## Metadata

- **Class**: BP_VRFunctionLibrary
- **ParentClass**: BlueprintFunctionLibrary

---

## Components

(None)

---

## Variables

(None)

---

## Functions

- GetHeldByHand -> EControllerHand ( LocalMotionController: BP_MyMotionController_C )

---

## Graph Inventory

- **Function**: 1

---

## Graph Logic

### [Function] GetHeldByHand

**Trace Start: Get Held by Hand** (FunctionEntry)

```blueprint
* Event Get Held by Hand
    * Return(WhichHand=Select(Index=(MotionSource == Left), Options={}))
```
---



---

## References

- /Game/VRGamePlay/Player/VROps/BP_MyMotionController.BP_MyMotionController_C (BP_MyMotionController_C) // GetHeldByHand :: Get Held by Hand.LocalMotionController
- /Script/HeadMountedDisplay.MotionControllerComponent (MotionControllerComponent) // GetHeldByHand :: Get MotionSource.self

