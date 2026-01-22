# 碰撞设计（以 CSV 为准）

本文用于在 UE 里配置 Collision Channel / Collision Profile，并作为后续代码改造的依据。
所有表格内容以 `Docs/Collision/` 目录下 3 个 CSV 为“唯一真相”，不要在文档里手改表格而不改 CSV：
- `Trace Channels-Collision Channel.csv`：Channel 定义与语义（Ignore/Overlap/Block 的含义）
- `Trace Channels-Collision Profile.csv`：各 Collision Profile 的响应矩阵
- `Trace Channels-Profile Reuse.csv`：同一实体在不同状态下复用哪个 Profile

## 设计原则
- 运行时切换“状态”只切换 Collision Profile；尽量避免逐个 ResponseToChannel 手动改。
- 抓取/投射物检测都走固定的 Trace Channel（`Trace_Grab` / `Trace_Projectile`）。
- 为“玩家手”单独定义 Object Channel（`Obj_PlayerHand`），用于背包/弓弦等交互区域的检测，避免用 Tag 分支。

## Channel（来自 Trace Channels-Collision Channel.csv）

### Trace Channels
| Name | Description | Ignore 语义 | Overlap 语义 | Block 语义 |
|---|---|---|---|---|
| Visibility | 暂无 |  |  |  |
| Camera | 暂无 |  |  |  |
| Trace_Grab | 寻找可抓取的物体 |  | 可被抓取 | 暂无 |
| Trace_Projectile | 寻找可被箭射中的物体 | 不可被射中 | 暂无 | 可被射中 |

### Object Channels
| Name | Description |
|---|---|
| WorldStatic | 场景中一切不动的物体 |
| WorldDynamic | 场景中会动但不模拟物理的物体 |
| Pawn | 玩家和敌人的胶囊体 |
| PhysicsBody | 场景中一切模拟物理的物体 |
| Vehicle | 暂无 |
| Destructible | 暂无 |
| Obj_PlayerHand | 玩家的手 |

## Collision Profile（来自 Trace Channels-Collision Profile.csv）

说明：下面每个 Profile 都列出 `Collision Enabled`、`Object Type`，以及对各 Channel 的 Response。

### Profile_PlayerCapsule
- Collision Enabled: `Query And Physics`
- Object Type: `Pawn`

| Channel | Response |
|---|---|
| Trace_Grab | Ignore |
| Trace_Projectile | Ignore |
| WorldStatic | Block |
| WorldDynamic | Block |
| Pawn | Ignore |
| PhysicsBody | Block |
| Obj_PlayerHand | Ignore |

### Profile_PlayerHand
- Collision Enabled: `Query Only`
- Object Type: `Obj_PlayerHand`

| Channel | Response |
|---|---|
| Trace_Grab | Ignore |
| Trace_Projectile | Ignore |
| WorldStatic | Overlap |
| WorldDynamic | Overlap |
| Pawn | Ignore |
| PhysicsBody | Overlap |
| Obj_PlayerHand | Ignore |

### Profile_PlayerBackpack
- Collision Enabled: `Query Only`
- Object Type: `WorldDynamic`

| Channel | Response |
|---|---|
| Trace_Grab | Ignore |
| Trace_Projectile | Ignore |
| WorldStatic | Ignore |
| WorldDynamic | Ignore |
| Pawn | Ignore |
| PhysicsBody | Ignore |
| Obj_PlayerHand | Overlap |

### Profile_EnemyCapsule
- Collision Enabled: `Query And Physics`
- Object Type: `Pawn`

| Channel | Response |
|---|---|
| Trace_Grab | Ignore |
| Trace_Projectile | Ignore |
| WorldStatic | Block |
| WorldDynamic | Block |
| Pawn | Ignore |
| PhysicsBody | Ignore |
| Obj_PlayerHand | Ignore |

### Profile_EnemyMesh_Alive
- Collision Enabled: `Query And Physics`
- Object Type: `PhysicsBody`

| Channel | Response |
|---|---|
| Trace_Grab | Ignore |
| Trace_Projectile | Block |
| WorldStatic | Block |
| WorldDynamic | Block |
| Pawn | Ignore |
| PhysicsBody | Block |
| Obj_PlayerHand | Ignore |

### Profile_EnemyMesh_Ragdoll
- Collision Enabled: `Query And Physics`
- Object Type: `PhysicsBody`

| Channel | Response |
|---|---|
| Trace_Grab | Block |
| Trace_Projectile | Block |
| WorldStatic | Block |
| WorldDynamic | Block |
| Pawn | Ignore |
| PhysicsBody | Block |
| Obj_PlayerHand | Ignore |

### Profile_Grabbable_Physics
- Collision Enabled: `Query And Physics`
- Object Type: `PhysicsBody`

| Channel | Response |
|---|---|
| Trace_Grab | Block |
| Trace_Projectile | Block |
| WorldStatic | Block |
| WorldDynamic | Block |
| Pawn | Ignore |
| PhysicsBody | Block |
| Obj_PlayerHand | Ignore |

### Profile_BowStringCollision
- Collision Enabled: `Query Only`
- Object Type: `WorldDynamic`

| Channel | Response |
|---|---|
| Trace_Grab | Ignore |
| Trace_Projectile | Ignore |
| WorldStatic | Ignore |
| WorldDynamic | Ignore |
| Pawn | Ignore |
| PhysicsBody | Ignore |
| Obj_PlayerHand | Overlap |

### Profile_Arrow_Stuck
- Collision Enabled: `Query Only`
- Object Type: `WorldDynamic`

| Channel | Response |
|---|---|
| Trace_Grab | Block |
| Trace_Projectile | Ignore |
| WorldStatic | Ignore |
| WorldDynamic | Ignore |
| Pawn | Ignore |
| PhysicsBody | Ignore |
| Obj_PlayerHand | Ignore |

### Profile_StasisPoint_Fired
- Collision Enabled: `Query Only`
- Object Type: `WorldDynamic`

| Channel | Response |
|---|---|
| Trace_Grab | Ignore |
| Trace_Projectile | Ignore |
| WorldStatic | Ignore |
| WorldDynamic | Overlap |
| Pawn | Ignore |
| PhysicsBody | Overlap |
| Obj_PlayerHand | Ignore |

### Profile_Star_Finger
- Collision Enabled: `Query Only`
- Object Type: `WorldDynamic`

| Channel | Response |
|---|---|
| Trace_Grab | Ignore |
| Trace_Projectile | Ignore |
| WorldStatic | Ignore |
| WorldDynamic | Overlap |
| Pawn | Ignore |
| PhysicsBody | Ignore |
| Obj_PlayerHand | Ignore |

### Profile_Star_Other
- Collision Enabled: `Query Only`
- Object Type: `WorldDynamic`

| Channel | Response |
|---|---|
| Trace_Grab | Ignore |
| Trace_Projectile | Ignore |
| WorldStatic | Ignore |
| WorldDynamic | Overlap |
| Pawn | Ignore |
| PhysicsBody | Ignore |
| Obj_PlayerHand | Ignore |

## Profile 复用（来自 Trace Channels-Profile Reuse.csv）

| 实体/状态 | 使用的 Profile |
|---|---|
| Bow_Body | Profile_Grabbable_Physics |
| Bow_String | NoCollision |
| Arrow_Idle | Profile_Grabbable_Physics |
| Arrow_Nocked | NoCollision |
| Arrow_Flying | NoCollision |
| StasisPoint_Held | NoCollision |
| Star_Main | NoCollision |

## 现状扫描 (C++)
- PC grab: LineTraceSingle, channel = GrabTraceChannel (Source/VRTest/Private/Game/BasePCPlayer.cpp)
- VR grab: SphereTraceMultiForObjects, object types = WorldDynamic/PhysicsBody (Source/VRTest/Private/Grabber/VRGrabHand.cpp)
- GravityGloves/Stasis target: FindActorsInCone (Source/VRTest/Private/Tools/GameUtils.cpp + VRGrabHand/VRStasisFireMonitor)
- Bow string overlap: OverlapAllDynamic (Source/VRTest/Private/Grabbee/Bow.cpp)
- Arrow states: Idle=IgnoreOnlyPawn, Nocked/Flying=NoCollision, Stuck=OverlapAllDynamic + Block Visibility (Source/VRTest/Private/Grabbee/Arrow.cpp)
- Enemy: Alive mesh=CharacterMesh + Block Visibility; Dead mesh=IgnoreOnlyPawn + SimulatePhysics (Source/VRTest/Private/Game/BaseEnemy.cpp)
- Hand/backpack triggers: QueryOnly + OverlapAll (Source/VRTest/Private/Game/BasePCPlayer.cpp / BaseVRPlayer.cpp)
- StarDraw: FingerPoint/OtherStar QueryOnly Overlap; MainStar NoCollision (Source/VRTest/Private/Skill/StarDraw*.cpp)
- StasisPoint: OverlapAllDynamic, fired toggles QueryOnly/NoCollision (Source/VRTest/Private/Skill/Stasis/StasisPoint.cpp)

## 实现注意事项（后续改代码时遵守）
- VR 抓取候选收集：不要用 Sweep（会被第一个 Block 终止、拿不到多个候选）；用 `GetWorld()->OverlapMultiByChannel(Trace_Grab)` 收集，再按 `IGrabbable::CanBeGrabbedBy` / 距离等规则筛选最近目标。
- PC 抓取：如果继续用 `LineTraceByChannel(Trace_Grab)`，目标需要对 `Trace_Grab` 为 `Block` 才能命中（本 CSV 的抓取物 Profile 已配置为 `Block`）。
- 背包逻辑迁移（TODO）：目前背包检测由“手”来做；若统一用 `Obj_PlayerHand` 做检测，建议把背包 enter/exit 的判定逻辑迁移到 `ABaseVRPlayer`（由玩家统一处理左右手的 overlap），避免到处查 Tag。
