// Skill/Stasis/StasisPoint.h

#pragma once

#include "CoreMinimal.h"
#include "FakePhysicsHandleActor.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "IStasisable.h"
#include "Grabber/IGrabbable.h"
#include "StasisPoint.generated.h"

class USceneComponent;
class UPrimitiveComponent;

/**
 * Stasis Point actor that can be grabbed and thrown to apply stasis effect
 * Inherits from FakePhysicsHandleActor and implements custom grab interface
 */
UCLASS()
class VRTEST_API AStasisPoint : public AFakePhysicsHandleActor, public IStasisable, public IGrabbable
{
    GENERATED_BODY()

public:
    AStasisPoint();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* Sphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNiagaraComponent* Niagara;

    // Variables
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stasis")
    USceneComponent* Target;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stasis")
    float TimeToStasis;

    /**
     * 发射定身球：由定身球内部自行进行“SphereOverlap + 角度筛选 + IStasisable 筛选”，并决定追踪目标。
     * - PC：调用端传入 Origin=CameraLocation, AimDirection=CameraForward
     * - VR：调用端传入 Origin=HandLocation, AimDirection=LastVelocityDirection
     * - 若未找到目标：按 AimDirection 直飞到 MaxNoTargetDistance，并在 NoTargetLifeSeconds 后自毁
     */
    UFUNCTION(BlueprintCallable, Category = "Stasis", meta=(WorldContext="WorldContextObject"))
    void Fire(
        UObject* WorldContextObject,
        const FVector& Origin,
        const FVector& AimDirection,
        const FVector& InitVelocity,
        float DetectionRadius,
        float DetectionAngleDegrees,
        const TArray<AActor*>& IgnoreActors,
        float MaxNoTargetDistance = 200000.0f,
        float NoTargetLifeSeconds = 10.0f
    );

    UFUNCTION(BlueprintCallable, Category = "Stasis")
    void EnterFollowingMode();

    UFUNCTION(BlueprintCallable, Category = "Stasis")
    void EnterTrackMode();
    
    // ==================== IGrabbable 接口实现 ====================
    
    virtual EGrabType GetGrabType_Implementation() const override;
    virtual UPrimitiveComponent* GetGrabPrimitive_Implementation() const override;
    virtual bool CanBeGrabbedBy_Implementation(const UPlayerGrabHand* Hand) const override;
    virtual bool CanBeGrabbedByGravityGlove_Implementation() const override;
    virtual bool SupportsDualHandGrab_Implementation() const override;
    
    virtual void OnGrabbed_Implementation(UPlayerGrabHand* Hand) override;
    virtual void OnReleased_Implementation(UPlayerGrabHand* Hand) override;
    
    virtual void OnGrabSelected_Implementation() override;
    virtual void OnGrabDeselected_Implementation() override;

protected:
    // Event handlers
    UFUNCTION()
    void OnSphereBeginOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

private:
    // Physics parameters
    float SpringStiffness;
    float Damping;
    float SpringForceMin;
    bool bGrabbed = false;

    /** 当前抓取此定身球的手（用于碰撞忽略与状态恢复） */
    UPROPERTY(Transient)
    TObjectPtr<UPlayerGrabHand> HoldingHand = nullptr;

    /** 握持/发射阶段：关闭自身碰撞并忽略与玩家当前手持物体的碰撞 */
    void ApplyHeldCollisionRules();

    /** 发射后：恢复自身碰撞（并尽量恢复忽略规则） */
    void RestorePostFireCollisionRules();

    /** 选择要追踪的组件（仅用于“普通可定身物体”，不做尸体/复杂角色特殊处理） */
    static USceneComponent* ChooseTrackComponent(AActor* TargetActor);

    /** 当前“无目标直飞”自毁定时器 */
    FTimerHandle NoTargetDestroyTimerHandle;

    /**
     * 定身目标检测使用的 ObjectTypes（OverlapMultiByObjectType）。
     * 默认包含：WorldDynamic / Pawn。
     * 之所以放在定身球内部，是为了避免在 PC/VR 两侧重复配置。
     */
    UPROPERTY(EditDefaultsOnly, Category = "Stasis|Targeting")
    TArray<TEnumAsByte<EObjectTypeQuery>> TargetingObjectTypes;

    /** 内部：根据发射上下文查找可定身目标（按角度优先） */
    AActor* FindStasisTarget(
        UObject* WorldContextObject,
        const FVector& Origin,
        const FVector& AimDirection,
        float DetectionRadius,
        float DetectionAngleDegrees,
        const TArray<AActor*>& IgnoreActors
    ) const;

    /** 内部：无目标时设置远点并启动超时自毁 */
    void StartNoTargetFlight(const FVector& Origin, const FVector& AimDirection, float MaxNoTargetDistance, float NoTargetLifeSeconds);
};
