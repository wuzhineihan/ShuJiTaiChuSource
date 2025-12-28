# BPI_CommonInterface

**Asset Path**: `/Game/enemy_ai_test/ai_enmey/Goap_AI/interface/BPI_CommonInterface.BPI_CommonInterface`

---

## Interface Functions

- AreYouAlive -> void ( Alive?: bool )
- AttachWeapon -> void ( WeaponRef: AActor*, SocketName: FName )
- GetAudio -> void ( AudioRef: UAudioComponent* )
- GetPatrolState -> void ( NeedToPatrol: bool )
- GetSleepState -> void ( IsSleep: bool )
- SetAnimationMode -> void ( TrueForSequenceMode: bool, AnimSequence: UAnimSequence* )
- SetWidgetComponent -> void ( WidgetComponent: UEnemyWidgetComponent_C* )
- WhoAreYou -> void ( MyName: FName )

---

## References

- /Game/ui/EnemyState/EnemyWidgetComponent.EnemyWidgetComponent_C (EnemyWidgetComponent_C) // SetWidgetComponent :: Set Widget Component.WidgetComponent

