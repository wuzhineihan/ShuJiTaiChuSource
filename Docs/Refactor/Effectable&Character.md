我决定直接重新写一遍游戏逻辑，而不是在原有逻辑上面改动，首先我们不管roadmap，按照我的思路重构逻辑，但你也要检查我的重构逻辑是否合理。我将原有游戏逻辑分成了几个模块，我们先从Effect模块和Game模块入手。

# Effect模块：

首先定义IEffectable，作为施加影响（伤害等）的接口，当拥有AliveComponent的类被施加影响时，接口函数实现会调用AliveComponent组件进行伤害逻辑的处理。当其他实现了IEffectable接口的类（不拥有AliveComponent）被施加影响时，如可以打碎的水缸，接口函数实现会进行相应的处理（如破碎）。

接口定义了ApplyEffect函数（无返回值），ApplyEffect函数的参数是一个叫做Effect的结构体，其中定义了施加的影响属性，包括：
- EffectTypeEnum[] EffectTypes, 
- float Amount, 
- float Duration,
- actor causer, 
- BaseCharacter instigator.

Enum EffectTypeEnum枚举定义了所有影响的类型，包括：
- Arrow
- Smash
- Melee
- Fire
- Stasis

定义AliveComponent组件，作为所有能够受伤的人/动物搭载的组件，有如下变量/函数：

- float HP（初始等于MaxHP）
- float MaxHP
- 多播委托（可被蓝图调用） Dead（无参数无返回值）：当HP=0时，调用此委托
- void DecreaseHP(float DecreaseAmount)
- void IncreaseHP(float IncreaseAmount)
- bool SetHP(float TargetHP)：返回是否set成功
- float GetHP(void)

定义FallDamageComponent组件（不继承AliveComponent，作为兄弟组件）：
- 依赖：需要在BeginPlay时获取Owner身上的AliveComponent。
- 逻辑：利用Owner（Character）的OnLanded事件来检测落地。当落地时，根据Z轴速度计算伤害，并调用AliveComponent->DecreaseHP。
- 变量：暴露摔落阈值与伤害系数给蓝图。

定义AutoRecoverComponent组件（不继承AliveComponent，作为兄弟组件）：
- 依赖：需要在BeginPlay时获取Owner身上的AliveComponent。
- 逻辑：在Tick中每帧恢复血量，调用AliveComponent->IncreaseHP。
- 变量：暴露回血速度给蓝图。

# Game模块：

将BaseCharacter作为所有角色（玩家，敌人）的基类。拥有的函数/逻辑如下：
- 有一个AliveComponent
- SceneComponent GetTrackOrigin()；默认返回CapsuleComponent，作用是返回AI视力的刺激点，子类可重写
- 对于每个具体的EffectTypeEnum（Arrow，Melee，Smash，Fire，Stasis），都有一个TakeEffect函数，如对于Arrow有TakeArrowEffect。这些函数的参数都是Effect结构体，子类可重写。
- 继承IEffectable接口，并提供默认实现，默认实现为：对于传入Effect结构体当中的EffectTypes数组当中的每一个元素，根据其EffectTypeEnum调用对应的TakeEffect函数。

BaseEnemy:BaseCharacter，用作所有敌人的基类

BasePlayer:BaseCharacter，拥有FallDamageComponent和AutoRecoverComponent，用作所有玩家类的基类（PC/VR）