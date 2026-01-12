#include "AI/Component/EventBusComponent.h"
#include "Logging/LogMacros.h"

// 定义 EventBus 的日志类别
DEFINE_LOG_CATEGORY_STATIC(LogEventBus, Log, All);

UEventBusComponent::UEventBusComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bDebugMode = false;
}

void UEventBusComponent::BroadcastEvent(FGameplayTag EventTag, UObject *Payload)
{
    if (!EventTag.IsValid())
    {
        UE_LOG(LogEventBus, Warning, TEXT("EventBus::BroadcastEvent - Invalid Tag ignored on Actor: %s"), *GetOwner()->GetName());
        return;
    }

    // 1. 递归卫士 (Recursion Guard)
    BroadcastDepth++;
    if (BroadcastDepth > MAX_BROADCAST_DEPTH)
    {
        UE_LOG(LogEventBus, Error, TEXT("EventBus::BroadcastEvent - Recursion limit reached (%d) for Tag [%s]! Possible infinite loop."), MAX_BROADCAST_DEPTH, *EventTag.ToString());
        BroadcastDepth--; // Unwind
        return;
    }
    // 作用域结束时自动递减
    // 注意: 虽然 TGuardValue 更整洁，但手动递增/递减在这里能更清晰地展示逻辑流。
    // 我们使用 RAII 辅助类来确保安全性，防止未来扩展代码时因 return/exception 导致计数器未复位。
    // 目前简单的递减还可以，但为了稳健性还是这样做。
    struct FDepthGuard
    {
        int32 &Depth;
        FDepthGuard(int32 &InDepth) : Depth(InDepth) {}
        ~FDepthGuard() { Depth--; }
    } Guard(BroadcastDepth);

    // 2. 调试追踪
#if WITH_EDITOR || !UE_BUILD_SHIPPING
    if (bDebugMode)
    {
        const int32 NativeCount = EventListeners.Contains(EventTag) ? 1 : 0; // 简化计数，真实计数需要检查委托
        UE_LOG(LogEventBus, Log, TEXT("[%s] Broadcasting Event: %s | Payload: %s"), *GetOwner()->GetName(), *EventTag.ToString(), Payload ? *Payload->GetName() : TEXT("None"));

        // 重用详细的 Dump 调用逻辑
        DumpListenersForEvent(EventTag);
    }
#endif

    // 3. 原生广播
    if (FOnGameplayEventNative *NativeDelegate = EventListeners.Find(EventTag))
    {
        if (NativeDelegate->IsBound())
        {
            NativeDelegate->Broadcast(EventTag, Payload);
        }
    }
}

FDelegateHandle UEventBusComponent::RegisterNativeListener(FGameplayTag EventTag, const FOnGameplayEventNative::FDelegate &Listener, const FString &DebugName)
{
    if (!EventTag.IsValid())
    {
        return FDelegateHandle();
    }

    FOnGameplayEventNative &Delegate = EventListeners.FindOrAdd(EventTag);
    FDelegateHandle Handle = Delegate.Add(Listener);

#if WITH_EDITOR || !UE_BUILD_SHIPPING
    AddDebugListener(EventTag, Handle, Listener, DebugName);
#endif

    return Handle;
}

FDelegateHandle UEventBusComponent::RegisterNativeListenerOnce(FGameplayTag EventTag, const FOnGameplayEventNative::FDelegate &Listener, const FString &DebugName)
{
    if (!EventTag.IsValid())
    {
        return FDelegateHandle();
    }

    // Create a shared pointer to hold the handle (chicken-and-egg problem solution)
    TSharedPtr<FDelegateHandle> HandlePtr = MakeShared<FDelegateHandle>();

    // Create the Proxy Lambda
    FOnGameplayEventNative::FDelegate ProxyDelegate = FOnGameplayEventNative::FDelegate::CreateLambda(
        [this, EventTag, Listener, HandlePtr](FGameplayTag Tag, UObject *Payload)
        {
            // 1. Execute User Logic (Safe check)
            if (Listener.IsBound())
            {
                Listener.Execute(Tag, Payload);
            }

            // 2. Self Unregister
            if (HandlePtr.IsValid() && HandlePtr->IsValid())
            {
                this->UnregisterNativeListener(EventTag, *HandlePtr);
            }
        });

    // 注册代理并存储句柄
    // 我们传递一个修改后的调试名称以表明它是 OneShot 包装器
    FString ProxyDebugName = DebugName + TEXT(" (OneShot)");
    *HandlePtr = this->RegisterNativeListener(EventTag, ProxyDelegate, ProxyDebugName);

    return *HandlePtr;
}

void UEventBusComponent::UnregisterNativeListener(FGameplayTag EventTag, FDelegateHandle Handle)
{
    if (FOnGameplayEventNative *Delegate = EventListeners.Find(EventTag))
    {
        Delegate->Remove(Handle);
    }

#if WITH_EDITOR || !UE_BUILD_SHIPPING
    RemoveDebugListener(EventTag, Handle);
#endif
}

void UEventBusComponent::UnregisterAllListenersForEvent(FGameplayTag EventTag)
{
    EventListeners.Remove(EventTag);

#if WITH_EDITOR || !UE_BUILD_SHIPPING
    DebugListenerMap.Remove(EventTag);
#endif
}

void UEventBusComponent::DumpListenersForEvent(FGameplayTag EventTag)
{
#if WITH_EDITOR || !UE_BUILD_SHIPPING
    if (TArray<FEventBusDebugListenerInfo> *List = DebugListenerMap.Find(EventTag))
    {
        UE_LOG(LogEventBus, Log, TEXT("--- Listeners for [%s] ---"), *EventTag.ToString());
        for (const FEventBusDebugListenerInfo &Info : *List)
        {
            FString ObjName = Info.ListenerObject.IsValid() ? Info.ListenerObject->GetName() : TEXT("DEAD_OBJECT");
            // 如果手动提供了 DebugName，则使用它，否则回退到对象名称
            FString DisplayName = Info.DebugName;

            UE_LOG(LogEventBus, Log, TEXT("  - [Object: %s] | [Ref: %s]"), *ObjName, *DisplayName);
        }
        UE_LOG(LogEventBus, Log, TEXT("--------------------------"));
    }
    else
    {
        UE_LOG(LogEventBus, Log, TEXT("No traced listeners for [%s] (or Debug info missing)"), *EventTag.ToString());
    }
#endif
}

#if WITH_EDITOR || !UE_BUILD_SHIPPING
void UEventBusComponent::AddDebugListener(FGameplayTag EventTag, FDelegateHandle Handle, const FOnGameplayEventNative::FDelegate &Listener, const FString &DebugName)
{
    FEventBusDebugListenerInfo Info;
    Info.Handle = Handle;
    Info.ListenerObject = Listener.GetUObject();
    Info.DebugName = DebugName;

    // 如果没有提供手动名称且我们有对象，则使用对象名称
    if (Info.DebugName == TEXT("NativeListener") && Info.ListenerObject.IsValid())
    {
        Info.DebugName = Info.ListenerObject->GetName();
    }

    DebugListenerMap.FindOrAdd(EventTag).Add(Info);
}

void UEventBusComponent::RemoveDebugListener(FGameplayTag EventTag, FDelegateHandle Handle)
{
    if (TArray<FEventBusDebugListenerInfo> *List = DebugListenerMap.Find(EventTag))
    {
        List->RemoveAll([&](const FEventBusDebugListenerInfo &Info)
                        { return Info.Handle == Handle; });
    }
}
#endif
