# BP_DamageStruct

**Asset Path**: `/Game/VRGamePlay/Combat/DamageType/BP_DamageStruct.BP_DamageStruct`

---

## Metadata

- **Class**: BP_DamageStruct
- **ParentClass**: DamageType
- **Interfaces**:
  - BPI_GetDamageInfo_C

---

## Components

(None)

---

## Variables

- DamageType : E_DamageType (Public) // Damage Type

---

## Functions

- GetDamgeType (Interface Implementation) -> E_DamageType

---

## Graph Inventory

- **Interface**: 1

---

## Graph Logic

### [Interface] GetDamgeType

**Trace Start: Get Damge Type** (FunctionEntry)

```blueprint
* Event Get Damge Type
    * Return(DamageType=DamageType)
```
---



---

## References

- /Game/VRGamePlay/Combat/DamageType/BPI_GetDamageInfo.BPI_GetDamageInfo_C (BPI_GetDamageInfo_C) // Implemented Interface

