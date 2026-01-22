# 项目记忆

保持简洁,记录当前代码行为要点.

## 项目基本信息
- 类型: VR潜行战斗游戏(PC/VR)
- 引擎: UE 5.4
- 语言: C++ + 蓝图
- 阶段: 蓝图迁移到C++的重构
- 协作: BP2AI 导出蓝图文档(BP2AIExport)

## 目录结构(核心)
Source/VRTest/
- Public/Tools 工具
- Public/Skill 技能
- Public/Grabber 抓取
- Public/Grabbee 可抓取物体
- Public/Game 玩家/设置/库存
- Public/Effect 效果/生命
- Private 实现
Docs/Refactor/

## 系统概况
- 效果/生命: IEffectable/FEffect/EEffectType + Alive/AutoRecover/FallDamage
- Grabber: UPlayerGrabHand(PhysicsHandle+Inventory注入), UPCGrabHand, UVRGrabHand(重力手套/背包区)
- Grabbee: AGrabbeeObject(双手/高亮/LaunchTowards), AGrabbeeWeapon(WeaponSnap)
- Bow/Arrow: ABow(弓身WeaponSnap/弓弦Custom/自动搭箭), AArrow(Idle/Nocked/Flying/Stuck/命中LineTrace)
- Game: ABasePlayer(双手PhysicsHandle/永久弓), ABasePCPlayer(射线目标/TryThrow), ABaseVRPlayer(Grip抓取)
- Skill: USkillAsset(轨迹映射+策略), UPlayerSkillComponent(学习+绘制+策略执行), AStarDrawManager(圆柱投影+other_stars)
- Stasis: AStasisPoint + AStasisSkillStrategy + AVRStasisFireMonitor(锥形选目标)
- Tools: UGameUtils::FindActorsInCone

## 约定/注意
- Grabber/Grabbee 严格分离; 统一 PhysicsHandle
- Tags: player_hand / player_backpack / other_stars
- 背包只存箭,弓永久持有

## 进度
- 已完成: 抓取系统,弓箭,基础Effect/生命,技能框架
- 待完善: 更多技能效果,敌人AI/GOAP,场景交互/解谜,VR弓交互回归测试

---
最后更新: 2026-01-19
