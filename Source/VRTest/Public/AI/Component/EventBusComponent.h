#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "EventBusComponent.generated.h"

// 定义用于 C++ 的原生多播委托
// 参数: EventTag, Payload Object
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGameplayEventNative, FGameplayTag, UObject *);

// 定义调试信息结构体
#if WITH_EDITOR || !UE_BUILD_SHIPPING
struct FEventBusDebugListenerInfo
{
    TWeakObjectPtr<UObject> ListenerObject;
    FString DebugName;
    FDelegateHandle Handle;

    bool operator==(const FDelegateHandle &OtherHandle) const
    {
        return Handle == OtherHandle;
    }
};
#endif

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class VRTEST_API UEventBusComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEventBusComponent();

    // 向所有监听者广播事件
    UFUNCTION(BlueprintCallable, Category = "EventBus")
    void BroadcastEvent(FGameplayTag EventTag, UObject *Payload = nullptr);

    // 注册原生 C++ 监听者
    // 返回一个句柄 (handle)，可用于稍后取消注册
    FDelegateHandle RegisterNativeListener(FGameplayTag EventTag, const FOnGameplayEventNative::FDelegate &Listener, const FString &DebugName = TEXT("NativeListener"));

    // 注册一次性监听者，触发一次后会自动取消注册
    FDelegateHandle RegisterNativeListenerOnce(FGameplayTag EventTag, const FOnGameplayEventNative::FDelegate &Listener, const FString &DebugName = TEXT("NativeListenerOnce"));

    // 使用句柄取消特定监听者的注册
    void UnregisterNativeListener(FGameplayTag EventTag, FDelegateHandle Handle);

    // 取消特定 Tag 下的所有监听者注册
    void UnregisterAllListenersForEvent(FGameplayTag EventTag);

    // --- 调试 ---

    // 如果为 true，将在广播时打印监听者日志
    UPROPERTY(EditAnywhere, Category = "EventBus|Debug")
    bool bDebugMode = false;

    // 将指定事件 Tag 的所有当前监听者打印到日志
    UFUNCTION(BlueprintCallable, Category = "EventBus|Debug")
    void DumpListenersForEvent(FGameplayTag EventTag);

protected:
    // 存储事件监听者的核心 Map
    TMap<FGameplayTag, FOnGameplayEventNative> EventListeners;

    // 递归卫士，用于防止广播过程中的死循环
    int32 BroadcastDepth = 0;
    static const int32 MAX_BROADCAST_DEPTH = 32;

#if WITH_EDITOR || !UE_BUILD_SHIPPING
    // 用于调试目的的影子 Map (Shadow Map)
    TMap<FGameplayTag, TArray<FEventBusDebugListenerInfo>> DebugListenerMap;

    void AddDebugListener(FGameplayTag EventTag, FDelegateHandle Handle, const FOnGameplayEventNative::FDelegate &Listener, const FString &DebugName);
    void RemoveDebugListener(FGameplayTag EventTag, FDelegateHandle Handle);
#endif
};
