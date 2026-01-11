# Event Bus Component Design (Local Actor Scope)

## 1. 核心目标
建立一个轻量级、基于 `UActorComponent` 的本地事件总线，用于解决 **Actor 内部** 各个组件（如武器、动画、属性、AI）之间的通信，避免直接强引用。

## 2. 核心设计决策

### A. 寻址方式 (Event Identity)
使用 **`FGameplayTag`** 作为事件的唯一标识。
*   **优点**: 灵活、分层（如 `Event.Damage.Taken`）、易于在编辑器配置、无需硬编码字符串或枚举。
*   **缺点**: 需要管理 Tag 字典。

### B. 载荷 (Payload) 数据传输
**方案**: 使用 `UObject*` 作为通用上下文 (Context Object) + 可选的参数包。
*   对于简单事件（如“跳跃开始”）：不需要 Payload。
*   对于复杂事件（如“收到伤害”）：传递一个 `UDamageContext` 对象或类似的结构。
*   **为什么不用 C++ 模板/Struct？**: 考虑到需要暴露给 Blueprint (BP) 以及通用存储，`UObject*` 兼容性最好。如果追求极致性能且纯 C++，可以使用 `FInstancedStruct` (StructUtils 插件)，但在基础架构中保持简单更好。

### C. 接口定义

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameplayEvent, FGameplayTag, EventTag, UObject*, Payload);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UEventBusComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // 发送事件
    UFUNCTION(BlueprintCallable, Category = "EventBus")
    void BroadcastEvent(FGameplayTag EventTag, UObject* Payload = nullptr);

    // 监听事件 (C++ 侧通常使用 Native Delegate，BP 侧使用 Dynamic Delegate)
    // 这里为了统一，内部可以用 TMap<FGameplayTag, FOnGameplayEvent> 
    // 或者提供更底层的 C++ 注册接口。
    
    // C++ 注册接口示例
    FDelegateHandle RegisterNativeListener(FGameplayTag EventTag, const FOnGameplayEventNative::FDelegate& Listener);
    void UnregisterNativeListener(FGameplayTag EventTag, FDelegateHandle Handle);
};
```

## 3. 待讨论细节
1.  **全局 vs 本地**: 我们的设计是 **本地 (Local)**，即每个 Character 挂一个 Bus，只处理自己的事件。
2.  **消息传递**: 是直接调用 Delegate (同步)，还是放入下一帧队列 (异步)？
    *   *建议*: **同步 (Synchronous)**。这更像是一个解耦的函数调用，逻辑流更清晰，调试更容易。

## 4. 目录结构
`Source/VRTest/Public/Core/Components/EventBusComponent.h`
`Source/VRTest/Private/Core/Components/EventBusComponent.cpp`

## 5. Payload 机制使用示例 (Example)

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
