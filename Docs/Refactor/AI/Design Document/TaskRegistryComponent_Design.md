# Task Registry Component Design (Local Actor Scope)

## 目录
- [Task Registry Component Design (Local Actor Scope)](#task-registry-component-design-local-actor-scope)
  - [目录](#目录)
  - [1. 核心目标](#1-核心目标)
  - [2. 核心设计决策](#2-核心设计决策)
    - [A. 任务注册中心 (Registry Pattern)](#a-任务注册中心-registry-pattern)
    - [B. 通信流向](#b-通信流向)
    - [C. 生命周期管理](#c-生命周期管理)
  - [3. 接口定义 (C++ API)](#3-接口定义-c-api)
  - [4. 目录结构](#4-目录结构)
  - [5. API 速查 (External API Summary)](#5-api-速查-external-api-summary)
    - [功能命令](#功能命令)
    - [查询](#查询)
    - [委托](#委托)
  - [6. 使用示例 (Example)](#6-使用示例-example)
    - [场景: AI 执行攻击任务](#场景-ai-执行攻击任务)
      - [Step 1: 执行者 (Executor) - MeleeComponent](#step-1-执行者-executor---meleecomponent)
      - [Step 2: 发布者 (Commander) - AIController](#step-2-发布者-commander---aicontroller)

## 1. 核心目标
构建一个 **任务注册与分发中心**，解耦“命令发布者”（AIController）与“命令执行者”（Actor Components）。
- **注册**：各个能力组件（如移动、攻击、交互）将自己的执行函数注册到此组件。
- **调度**：AIController 仅需通过 `GameplayTag` 查找并触发对应的逻辑，无需持有具体组件的引用。
- **反馈**：统一管理任务完成的通知信号。

## 2. 核心设计决策

### A. 任务注册中心 (Registry Pattern)
- 组件内部维护一个 `TMap<FGameplayTag, FTaskExecutionDelegate>`。
- **Key**: 任务标签 (如 `Task.Combat.Attack`, `Task.Move.GoTo`).
- **Value**: 指向具体执行函数的委托 (Delegate)。

### B. 通信流向
1.  **Setup (BeginPlay)**: Actor 身上的各个 Component (如 `MeleeComponent`) 调用 `RegisterTask` 将自己的函数绑定到特定 Tag。
2.  **Command**: AIController 调用 `ExecuteTask(Tag)`。
3.  **Execution**: `TaskRegistryComponent` 查找 Map，若存在则调用委托，返回 `true`。不存在则返回 `false`。
4.  **Completion**: 执行者完成任务后，调用 `NotifyTaskFinished`，组件对外广播 `OnTaskFinished` 委托，通知 AIController。

### C. 生命周期管理
- 任务的执行是异步的。`ExecuteTask` 仅负责启动任务。
- `OnTaskFinished` 委托用于通知任务结束。

## 3. 接口定义 (C++ API)

```cpp
// 任务执行委托 (单播，绑定具体功能函数)
// 可以根据需要添加 Payload 参数，暂时保持简单
DECLARE_DELEGATE_RetVal(bool, FTaskExecutionDelegate); 
// 或者如果是简单的触发: DECLARE_DELEGATE(FTaskExecutionDelegate); 
// 考虑到 execute 本身返回 bool 表示是否成功启动，委托本身可以返回 bool 表示内部逻辑是否接受

// 任务完成委托 (多播，通知 AIController)
// Payload: 完成的任务 Tag, 结果 (Success/Fail)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTaskFinishedCallback, FGameplayTag, TaskTag, bool, bSuccess);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UTaskRegistryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // --- 1. 任务注册 (供 Ability Components 使用) ---

    /**
     * 注册一个任务处理函数
     * @param TaskTag 任务标签
     * @param TaskDelegate 执行逻辑的委托 (通常绑定到 Component 的成员函数)
     */
    void RegisterTask(FGameplayTag TaskTag, FTaskExecutionDelegate TaskDelegate);

    /**
     * 取消注册
     */
    void UnregisterTask(FGameplayTag TaskTag);


    // --- 2. 任务执行 (供 AIController 使用) ---

    /**
     * 尝试执行某个任务
     * @param TaskTag 要执行的任务标签
     * @return true: 任务已注册并成功调用; false: 任务未注册
     */
    UFUNCTION(BlueprintCallable, Category = "AI|Task")
    bool ExecuteTask(FGameplayTag TaskTag);


    // --- 3. 任务反馈 (供 Ability Components 与 AIController 使用) ---

    /**
     * [对外广播] 任务完成委托
     * AIController 订阅此委托以接收任务结束通知
     */
    UPROPERTY(BlueprintAssignable, Category = "AI|Task")
    FOnTaskFinishedCallback OnTaskFinished;

    /**
     * [内部调用] 通知任务已完成
     * 执行者 (Executor) 在逻辑结束后调用此函数，触发 OnTaskFinished 广播
     * @param TaskTag 完成的任务 Tag
     * @param bSuccess 任务是否成功完成
     */
    UFUNCTION(BlueprintCallable, Category = "AI|Task")
    void NotifyTaskFinished(FGameplayTag TaskTag, bool bSuccess);

    // --- 4. 任务查询 (供 AIController 决策使用) ---

    /**
     * 获取所有已注册的任务标签
     * AIController 可在决策阶段调用此函数，查询当前 Pawn 具备哪些能力 (如: 能否攻击? 能否交互?)
     * @return 包含所有可用任务 Tag 的容器
     */
    UFUNCTION(BlueprintPure, Category = "AI|Task")
    FGameplayTagContainer GetRegisteredTasks() const;

private:
    // 任务注册表
    TMap<FGameplayTag, FTaskExecutionDelegate> TaskMap;
};
```

## 4. 目录结构
`Source/VRTest/Public/AI/Component/TaskRegistryComponent.h`
`Source/VRTest/Private/AI/Component/TaskRegistryComponent.cpp`

## 5. API 速查 (External API Summary)

### 功能命令
| 函数名 | 角色 | 说明 |
| :--- | :--- | :--- |
| **`ExecuteTask(Tag)`** | AIController | 尝试启动指定 Tag 的任务。返回是否成功启动。 |
| **`RegisterTask(Tag, Delegate)`** | Executor | (C++ Only) 将自身的功能函数注册到组件。 |
| **`NotifyTaskFinished(Tag, Success)`** | Executor | 任务完成后调用，触发广播。 |

### 查询
| 函数名 | 角色 | 说明 |
| :--- | :--- | :--- |
| **`GetRegisteredTasks()`** | AIController | 获取当前所有已注册的任务列表，用于状态判断和决策剪枝。 |

### 委托
| 事件名 | 角色 | 说明 |
| :--- | :--- | :--- |
| **`OnTaskFinished`** | AIController | 监听任务结束。Payload: `TaskTag`, `bSuccess`。 |

## 6. 使用示例 (Example)

### 场景: AI 执行攻击任务

#### Step 1: 执行者 (Executor) - MeleeComponent
在 `MeleeComponent` 中注册攻击逻辑，并在攻击结束后通知完成。

```cpp
// UMeleeComponent.cpp

void UMeleeComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // 1. 获取注册组件
    if (auto* TaskReg = GetOwner()->FindComponentByClass<UTaskRegistryComponent>())
    {
        // 2. 注册任务 "Task.Action.Attack"
        // 绑定到本类的 PerformAttack 函数
        TaskReg->RegisterTask(
            FGameplayTag::RequestGameplayTag("Task.Action.Attack"),
            FTaskExecutionDelegate::CreateUObject(this, &UMeleeComponent::PerformAttack)
        );
    }
}

// 具体的任务逻辑
bool UMeleeComponent::PerformAttack()
{
    if (bIsAttacking) return false; // 忙碌中，无法启动
    
    // 开始攻击逻辑 (播放动画等...)
    PlayAttackAnim();
    return true; // 成功启动
}

// 假设动画播完或者连招结束
void UMeleeComponent::OnAttackAnimFinished()
{
    // 3. 通知任务完成
    if (auto* TaskReg = GetOwner()->FindComponentByClass<UTaskRegistryComponent>())
    {
        TaskReg->NotifyTaskFinished(
            FGameplayTag::RequestGameplayTag("Task.Action.Attack"),
            true // Success
        );
    }
}
```

#### Step 2: 发布者 (Commander) - AIController
AIController 只要拿到 Actor，就可以请求任务并等待完成，无需知道有没有 `MeleeComponent`。

```cpp
// AEnemyAIController.cpp

void AEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    if (auto* TaskReg = InPawn->FindComponentByClass<UTaskRegistryComponent>())
    {
        // 1. 订阅完成信号
        TaskReg->OnTaskFinished.AddDynamic(this, &AEnemyAIController::HandleTaskFinished);
    }
}

void AEnemyAIController::ChooseNextAction()
{
    // ... 决策逻辑决定攻击 ...
    
    if (auto* TaskReg = GetPawn()->FindComponentByClass<UTaskRegistryComponent>())
    {
        // 2. 发出命令
        bool bStarted = TaskReg->ExecuteTask(FGameplayTag::RequestGameplayTag("Task.Action.Attack"));
        
        if (!bStarted)
        {
            // 任务启动失败 (比如组件不存在，或者冷却中)
            // 切换到其他状态
        }
        else
        {
            // 等待 HandleTaskFinished 回调...
        }
    }
}

void AEnemyAIController::HandleTaskFinished(FGameplayTag Tag, bool bSuccess)
{
    if (Tag == FGameplayTag::RequestGameplayTag("Task.Action.Attack"))
    {
        // 攻击结束，决策下一步
        ChooseNextAction();
    }
}
```
