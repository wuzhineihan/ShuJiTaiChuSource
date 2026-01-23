# Animation Controller Component Design (Local Actor Scope)

## 目录
- [Animation Controller Component Design (Local Actor Scope)](#animation-controller-component-design-local-actor-scope)
  - [目录](#目录)
  - [1. 核心目标](#1-核心目标)
  - [2. 核心设计决策](#2-核心设计决策)
    - [A. 动画资源完全来自于外界传递，自身作为中介不承担存储资源功能。](#a-动画资源完全来自于外界传递自身作为中介不承担存储资源功能)
    - [B. 通信机制与状态归属](#b-通信机制与状态归属)
    - [C. 动画通知](#c-动画通知)
    - [D. 接口定义 (C++ API)](#d-接口定义-c-api)
  - [3. 待讨论细节 (Pending Details)](#3-待讨论细节-pending-details)
  - [4. 目录结构](#4-目录结构)
  - [5. API 速查 (External API Summary)](#5-api-速查-external-api-summary)
    - [命令](#命令)
    - [查询](#查询)
  - [6. 使用示例 (Example)](#6-使用示例-example)
    - [场景: 近战组件发起攻击并处理被打断](#场景-近战组件发起攻击并处理被打断)

## 1. 核心目标
构建一个连接 AI/Gameplay 逻辑与底层动画系统的中间层，职责明确：
- **执行**：播放事件总线转递的动画蒙太奇。
- **广播**：统一对外广播“动画完成/被打断”等状态；
- **去中心**：仅作为播放动画蒙太奇的中间件，不参与其他的逻辑调用。
- **未来扩展**：可能会添加LevelSequence播放的功能

## 2. 核心设计决策

### A. 动画资源完全来自于外界传递，自身作为中介不承担存储资源功能。
- **输入**：动画蒙太奇资源、唯一标识Guid、该动作标签（比如攻击的话就是Attack标签，受击的话就是Hit标签）。
- **输出**：通过`Event.Anim.Finished`向事件总线发起通知，输出：当前的Guid，当前的动作标签，是否完整播放，是否被打断
  
### B. 通信机制与状态归属
- **关联凭证**：使用 `FGuid RequestID` 关联一次播放请求及其完成事件。
- **状态归属（关键）**：
  - 请求者长期订阅 `Event.Anim.Finished`，收到后按 `Payload.RequestID` 做自我过滤。
- **控制器职责**：
  - `UAnimationControllerComponent` 仅负责“执行与广播”，不保存业务回调、不维护多路映射。
  - 播放新动画前，控制器从 `AnimInstance` 读取 `CurrentMontageRequestID`（由 AnimInstance 在开播时记录）。若存在旧 ID，则：
    1.  停止当前蒙太奇。
    2.  立刻广播 `Event.Anim.Finished`，并依据当前情况组装payload。
  - 开始新动画，并将“新 ID”写入 `AnimInstance.CurrentMontageRequestID`，以便下一次切换时能正确中断并广播旧 ID。

**说明**：核心目标即是实现多个蒙太奇的播放而不造成互相影响

### C. 动画通知
- **通过接口通知**：为特定的动画通知定义相关接口，比如近战武器的开启碰撞体或者后续连招变招都可以通过动画通知实现 

### D. 接口定义 (C++ API)
```cpp
// 1) 结果载荷 (Output)
UENUM(BlueprintType)
enum class EAnimCompletionResult : uint8
{
    Finished,     // 完整播放
    Interrupted   // 被打断
};

UCLASS(BlueprintType)
class UAnimResultPayload : public UObject
{
    GENERATED_BODY()
public:
    // 当前的Guid
    UPROPERTY(BlueprintReadOnly, Category = "Result")
    FGuid RequestID;

    // 当前的动作标签
    UPROPERTY(BlueprintReadOnly, Category = "Result")
    FGameplayTag ActionTag;

    // 是否完整播放 / 是否被打断
    UPROPERTY(BlueprintReadOnly, Category = "Result")
    EAnimCompletionResult Result = EAnimCompletionResult::Finished;
};

// 2) 请求载荷 (Input)
UCLASS(BlueprintType)
class UAnimRequestPayload : public UObject
{
    GENERATED_BODY()
public:
    // 动画蒙太奇资源
    UPROPERTY(BlueprintReadWrite, Category = "Request", meta = (ExposeOnSpawn = true))
    UAnimMontage* Montage = nullptr;

    // 唯一标识Guid (若为空则内部生成)
    UPROPERTY(BlueprintReadWrite, Category = "Request", meta = (ExposeOnSpawn = true))
    FGuid RequestID;

    // 该动作标签 (用于上下文匹配)
    UPROPERTY(BlueprintReadWrite, Category = "Request", meta = (ExposeOnSpawn = true))
    FGameplayTag ActionTag;

    // 播放速率
    UPROPERTY(BlueprintReadWrite, Category = "Request")
    float PlayRate = 1.0f;

    // 起始段落名
    UPROPERTY(BlueprintReadWrite, Category = "Request")
    FName StartSection = NAME_None;
};

// 3) 控制器对外 API
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UAnimationControllerComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    // --- Commands (命令) ---

    /**
     * 执行动画播放
     * @param RequestInput  包含 资源、Tag、ID 及播放参数的请求载荷
     * @return              本次播放关联的 RequestID
     */
    UFUNCTION(BlueprintCallable, Category = "AI|Animation")
    FGuid PlayAnimation(UAnimRequestPayload* RequestInput);

    /**
     * 主动停止当前动画
     * 若当前有正在播放的蒙太奇，将导致广播 EAnimCompletionResult::Interrupted
     */
    UFUNCTION(BlueprintCallable, Category = "AI|Animation")
    void StopCurrentAnimation(float BlendOutTime = 0.2f);

    // --- Queries (查询) ---

    /** 获取当前正在播放的动作标签 (若无播放则返回 Empty) */
    UFUNCTION(BlueprintPure, Category = "AI|Animation")
    FGameplayTag GetCurrentActionTag() const;

    /** 获取当前蒙太奇播放位置（秒） */
    UFUNCTION(BlueprintPure, Category = "AI|Animation")
    float GetCurrentMontagePosition() const;

    /** 是否正在播放任何由该控制器管理的动画 */
    UFUNCTION(BlueprintPure, Category = "AI|Animation")
    bool IsPlaying() const;

protected:
    // --- Internal Handlers (内部处理) ---

    /**
     * 绑定到底层 AnimInstance 的 OnMontageEnded 委托。
     * 核心职责：捕获动画结束（自然完成或被打断），并广播 Event.Anim.Finished。
     * 注意：此函数由系统自动调用，不可被外部 Gameplay 逻辑直接调用。
     */
    UFUNCTION()
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};
```

## 3. 待讨论细节 (Pending Details)
- **多槽位蒙太奇**：多槽位播放，比如说上半身，下半身或者手臂分开来播放

## 4. 目录结构
`Source/VRTest/Public/AI/Component/AnimationControllerComponent.h`
`Source/VRTest/Private/AI/Component/AnimationControllerComponent.cpp`
`Source/VRTest/Public/AI/Data/AnimationConfigDataAsset.h`

## 5. API 速查 (External API Summary)

### 命令
| 函数名 | 说明 | 适用场景 |
| :--- | :--- | :--- |
| **`PlayAnimation`** | 播放指定蒙太奇 (需传入 Payload)，返回 RequestID。会自动打断当前播放。 | 发起攻击、受击硬直、闪避等动作播放。 |
| **`StopCurrentAnimation`** | 强制停止当前动画，并广播 `Interrupted` 结果。 | 强行打断动作（如死亡时），或重置状态。 |

### 查询
| 函数名 | 说明 | 适用场景 |
| :--- | :--- | :--- |
| **`GetCurrentActionTag`** | 返回当前正在播放的动作标签 (ActionTag)。若无动作则返回 Empty。 | 判断当前是在攻击还是受击，进行逻辑分流。 |
| **`GetCurrentMontagePosition`** | 返回当前蒙太奇的播放进度（秒）。 | 精确判定是否可以取消后摇，或触发连招。 |
| **`IsPlaying`** | 是否有任何由 Controller 管理的动画正在播放。 | 基础的状态判断。 |


## 6. 使用示例 (Example)

### 场景: 近战组件发起攻击并处理被打断

Requester（如 `UMeleeAttackComponent`）内部仅保存自己最近一次发起的攻击 `RequestID`，并订阅统一事件：

```cpp

```

