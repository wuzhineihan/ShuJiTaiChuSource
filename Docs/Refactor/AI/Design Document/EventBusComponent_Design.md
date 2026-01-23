# Event Bus Component Design (Local Actor Scope)

## 目录
- [Event Bus Component Design (Local Actor Scope)](#event-bus-component-design-local-actor-scope)
  - [目录](#目录)
  - [1. 核心目标](#1-核心目标)
  - [2. 核心设计决策](#2-核心设计决策)
    - [A. 寻址方式 (Event Identity)](#a-寻址方式-event-identity)
    - [B. 载荷 (Payload) 数据传输](#b-载荷-payload-数据传输)
    - [C. 接口定义 (C++ API)](#c-接口定义-c-api)
    - [D. 安全性与健壮性设计](#d-安全性与健壮性设计)
  - [3. 待讨论细节 (已归档)](#3-待讨论细节-已归档)
  - [4. 目录结构](#4-目录结构)
  - [5. API 速查 (External API Summary)](#5-api-速查-external-api-summary)
  - [6. 使用示例 (Example)](#6-使用示例-example)
    - [场景: 敌人受到攻击 (Event.Combat.DamageTaken)](#场景-敌人受到攻击-eventcombatdamagetaken)
      - [Step 1: 定义 Payload 数据对象](#step-1-定义-payload-数据对象)
      - [Step 2: 发送方 (Sender) - Component](#step-2-发送方-sender---component)
      - [Step 3: 接收方 (Receiver) - UI / AI](#step-3-接收方-receiver---ui--ai)

## 1. 核心目标
建立一个轻量级、基于 `UActorComponent` 的本地事件总线，用于解决 **Actor 内部** 各个组件（如武器、动画、属性、AI）之间的通信，避免直接强引用，同时也能够实现Actor和component之间的一部分通信。

## 2. 核心设计决策

### A. 寻址方式 (Event Identity)
使用 **`FGameplayTag`** 作为事件的唯一标识。 灵活、分层（如 `Event.Damage.Taken`）、易于在编辑器配置、无需硬编码字符串或枚举。在GameplayTag中所有事件均隶属于Event标签下。


### B. 载荷 (Payload) 数据传输
**方案**: 使用 `UObject*` 作为通用上下文 (Context Object) + 可选的参数包。
*   对于简单事件（如“跳跃开始”）：不需要 Payload。
*   对于复杂事件（如“收到伤害”）：传递一个 `UDamageContext` 对象或类似的结构。
*   **为什么不用 C++ 模板/Struct？**: 考虑到需要暴露给 Blueprint (BP) 以及通用存储，`UObject*` 兼容性最好。如果追求极致性能且纯 C++，可以使用 `FInstancedStruct` (StructUtils 插件)，但在基础架构中保持简单更好。

### C. 接口定义 (C++ API)

这个组件主要服务于 C++ 层的通信。虽然设计上考虑了蓝图扩展，但目前核心实现集中在高效的 C++ 原生委托上。

```cpp
// 1. 定义原生多播委托 (Native Multicast Delegate)
// 参数: EventTag (事件标签), Payload (上下文数据对象)
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGameplayEventNative, FGameplayTag, UObject*);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class VRTEST_API UEventBusComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEventBusComponent();

    /**
     * 向所有监听者广播事件
     * @param EventTag - 事件的唯一标识 Tag
     * @param Payload - (可选) 传递的数据载荷对象
     */
    UFUNCTION(BlueprintCallable, Category = "EventBus")
    void BroadcastEvent(FGameplayTag EventTag, UObject* Payload = nullptr);

    /**
     * 注册原生 C++ 监听者
     * @param EventTag - 要监听的事件 Tag
     * @param Listener - 绑定的委托函数
     * @param DebugName - (可选) 调试显示的名称，方便在 Log 中定位是谁注册的
     * @return FDelegateHandle - 用于后续取消注册的句柄
     */
    FDelegateHandle RegisterNativeListener(FGameplayTag EventTag, const FOnGameplayEventNative::FDelegate& Listener, const FString& DebugName = TEXT("NativeListener"));

    /**
     * 注册一次性监听者 (One-shot)
     * 分发一次事件后，会自动取消注册。实现上使用了 Lambda Proxy 模式。
     */
    FDelegateHandle RegisterNativeListenerOnce(FGameplayTag EventTag, const FOnGameplayEventNative::FDelegate& Listener, const FString& DebugName = TEXT("NativeListenerOnce"));

    /**
     * 取消特定的监听者
     */
    void UnregisterNativeListener(FGameplayTag EventTag, FDelegateHandle Handle);

    /**
     * 清空指定事件的所有监听者
     */
    void UnregisterAllListenersForEvent(FGameplayTag EventTag);

    // --- 调试与诊断 (Debugging) ---

    // 如果为 true，每次广播事件时会在 Output Log 打印详细的接收者列表
    UPROPERTY(EditAnywhere, Category = "EventBus|Debug")
    bool bDebugMode = false;

    // 手动打印指定 EventTag 当前所有的监听者名称和对象引用
    UFUNCTION(BlueprintCallable, Category = "EventBus|Debug")
    void DumpListenersForEvent(FGameplayTag EventTag);

protected:
    // 存储事件监听者的核心 Map (Tag -> Native Multicast Delegate)
    TMap<FGameplayTag, FOnGameplayEventNative> EventListeners;

    // 递归深度计数器
    int32 BroadcastDepth = 0;
    static const int32 MAX_BROADCAST_DEPTH = 32;

#if WITH_EDITOR || !UE_BUILD_SHIPPING
    // 用于调试的影子 Map (Shadow Map)，记录详细的监听者元数据
    // 仅在 Editor 或非 Shipping 构建中存在
    TMap<FGameplayTag, TArray<FEventBusDebugListenerInfo>> DebugListenerMap;

    // 内部调试辅助函数：添加监听者记录
    void AddDebugListener(FGameplayTag EventTag, FDelegateHandle Handle, const FOnGameplayEventNative::FDelegate& Listener, const FString& DebugName);
    
    // 内部调试辅助函数：移除监听者记录
    void RemoveDebugListener(FGameplayTag EventTag, FDelegateHandle Handle);
#endif
};
```

### D. 安全性与健壮性设计
1.  **递归卫士 (Recursion Guard)**:
    *   为了防止 A -> B -> A 这种无限事件循环导致的栈溢出，在 `BroadcastEvent` 内部实现了递归深度检测。
    *   **深度上限**: 默认为 32。超过此深度将打印 Error Log 并强制中断当前事件链。
    *   **实现**: 使用 RAII 模式 (`FDepthGuard` 结构体) 确保即使函数提前返回或异常退出，计数器也能正确复位。

2.  **调试影子 Map (Shadow Map Example)**:
    *   由于原生 C++ `TMulticastDelegate` 是不透明的（无法遍历获取绑定对象的名称），为了调试方便，我们在 `#if WITH_EDITOR` 宏下维护了一个并行的 `DebugListenerMap`。
    *   它记录了 `{Tag -> [对象名, 调试名, Handle]}` 的映射关系，仅用于 `DumpListenersForEvent` 和日志打印，不参与实际逻辑执行。发行版 (Shipping) 中会被完全剔除。


## 3. 待讨论细节 (已归档)
*   (已解决) 采用同步广播模式。
*   (已解决) 增加了一次性订阅接口 `RegisterNativeListenerOnce`。
*   (已解决) 增加了 Debug 影子列表与查询接口。

## 4. 目录结构
`Source/VRTest/Public/AI/Component/EventBusComponent.h`
`Source/VRTest/Private/AI/Component/EventBusComponent.cpp`

## 5. API 速查 (External API Summary)

| 函数名 | 说明 | 适用场景 |
| :--- | :--- | :--- |
| **`BroadcastEvent`** | 向所有监听者广播指定 Tag 的事件。 | 发送消息时。例如：角色死亡、受到伤害、任务完成。 |
| **`RegisterNativeListener`** | 注册一个持久的 C++ 监听函数，直到手动注销或对象销毁。 | 基础监听。例如：UI 组件监听血量变化，持续更新血条。 |
| **`RegisterNativeListenerOnce`** | 注册一次性监听者，触发一次后自动注销。 | 临时任务。例如：等待动作播放完毕的回调、等待下一个特定的状态切换。 |
| **`UnregisterNativeListener`** | 使用之前返回的 Handle 取消注册。 | 在组件销毁 `EndPlay` 或不再需要监听时清理。 |
| **`UnregisterAllListenersForEvent`** | 移除指定 Tag 下的所有监听者。 | 彻底重置某个事件状态，或在特殊清理流程中使用。 |
| **`DumpListenersForEvent`** | (Debug) 打印当前监听该事件的所有对象名称。 | 调试用。当事件触发没反应，或者触发了太多次时，检查是谁在监听。 |

## 6. 使用示例 (Example)

### 场景: 敌人受到攻击 (Event.Combat.DamageTaken)
当敌人受到攻击时，CombatComponent 发出事件，UI (显示飘字) 和 AI (播放受击动画/转向) 监听该事件。

#### Step 1: 定义 Payload 数据对象
为了传递伤害来源、数值、位置等信息，我们定义一个继承自 `UObject` 的数据类。建议使用 `UObject` 而非结构体，因为 `UObject` 可以方便地被蓝图引用和传递。

```cpp
// 建议放在 types 头文件中，如 GameDataTypes.h
UCLASS(BlueprintType)
class UDamagePayload : public UObject
{
    GENERATED_BODY()

public:
    // 我们可以添加一个简单的工厂方法来快速创建
    static UDamagePayload* Create(UObject* Outer, AActor* Attacker, float DamageAmount, FVector HitLocation)
    {
        UDamagePayload* Payload = NewObject<UDamagePayload>(Outer);
        Payload->Attacker = Attacker;
        Payload->DamageAmount = DamageAmount;
        Payload->HitLocation = HitLocation;
        return Payload;
    }

    UPROPERTY(BlueprintReadOnly)
    AActor* Attacker = nullptr;

    UPROPERTY(BlueprintReadOnly)
    float DamageAmount = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    FVector HitLocation = FVector::ZeroVector;
};
```

#### Step 2: 发送方 (Sender) - Component
在 `HealthComponent` 或 `CombatComponent` 中：

```cpp
void UHealthComponent::TakeDamage(float Damage, AActor* Attacker, FVector HitPos)
{
    // ... 扣血逻辑 ...

    // 组装 Payload
    UDamagePayload* Payload = UDamagePayload::Create(this, Attacker, Damage, HitPos);
    
    // 获取 EventBus 并广播
    if (UEventBusComponent* Bus = Owner->FindComponentByClass<UEventBusComponent>())
    {
        // Tag: "Event.Combat.DamageTaken"
        Bus->BroadcastEvent(FGameplayTag::RequestGameplayTag("Event.Combat.DamageTaken"), Payload);
    }
}
```

#### Step 3: 接收方 (Receiver) - UI / AI
在 `UIWidgetManager` 或 `AIController` 中：

```cpp
void UActorWidgetManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (UEventBusComponent* Bus = GetOwner()->FindComponentByClass<UEventBusComponent>())
    {
        // 绑定监听
        Bus->RegisterNativeListener(FGameplayTag::RequestGameplayTag("Event.Combat.DamageTaken"), 
            FOnGameplayEventNative::FDelegate::CreateUObject(this, &UActorWidgetManager::OnDamageTaken));
    }
}

void UActorWidgetManager::OnDamageTaken(FGameplayTag EventTag, UObject* Payload)
{
    // 转换 Payload
    if (UDamagePayload* DamageData = Cast<UDamagePayload>(Payload))
    {
        // 执行逻辑：显示飘字
        ShowDamageNumber(DamageData->DamageAmount, DamageData->HitLocation);
    }
}
```
