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

    // Custom Events
    UFUNCTION(BlueprintCallable, Category = "Stasis")
    void Fire(FVector InitVelocity, USceneComponent* TrackTarget);

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
};