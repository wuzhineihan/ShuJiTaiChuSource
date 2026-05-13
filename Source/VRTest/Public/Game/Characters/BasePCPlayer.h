// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "Game/Characters/BasePlayer.h"
#include "Game/CollisionConfig.h"
#include "BasePCPlayer.generated.h"
class UPCGrabHand;
class UCameraComponent;
class IGrabbable;
class AArrow;
class UPCClimbLadderComponent;
class UPCWindowVaultComponent;
class UPCActionPromptComponent;
class UPrimitiveComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGrabTargetChanged, AActor*, NewTarget, AActor*, OldTarget);

/**
 * PC 妯″紡鐜╁鍩虹被
 *
 * 鍖呭惈绗竴浜虹О鎽勫儚鏈哄拰鍙屾墜鎶撳彇閫昏緫銆?
 * 寮撶妯″紡鐢卞熀绫?bIsBowArmed 鎺у埗銆?
 */
UCLASS()
class VRTEST_API ABasePCPlayer : public ABasePlayer
{
	GENERATED_BODY()

public:
	ABasePCPlayer();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

public:
	virtual void Tick(float DeltaTime) override;

	// ==================== 缁勪欢 ====================
	
	/** 绗竴浜虹О鎽勫儚鏈?*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* FirstPersonCamera;

	/** 宸︽墜鎶撳彇缁勪欢锛圥C 鍏蜂綋绫诲瀷锛屼笌 BasePlayer::LeftHand 鎸囧悜鍚屼竴瀵硅薄锛?*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPCGrabHand* PCLeftHand;

	/** 鍙虫墜鎶撳彇缁勪欢锛圥C 鍏蜂綋绫诲瀷锛屼笌 BasePlayer::RightHand 鎸囧悜鍚屼竴瀵硅薄锛?*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPCGrabHand* PCRightHand;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* LeftHandCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* RightHandCollision;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* CameraCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPCClimbLadderComponent* PCClimbLadderComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPCWindowVaultComponent* PCWindowVaultComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPCActionPromptComponent* PCActionPromptComponent = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|CameraCollision", meta=(ClampMin="0.0"))
	float CameraCollisionRadius = 12.0f;

	// ==================== 鐩爣妫€娴嬮厤缃?====================

	/** 鎶撳彇灏勭嚎鏈€澶ц窛绂?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
	float MaxGrabDistance = 300.0f;

	/** 鎶撳彇妫€娴嬮€氶亾 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
	TEnumAsByte<ECollisionChannel> GrabTraceChannel = TCC_GRAB;

	/** 鏄惁缁樺埗鎶撳彇灏勭嚎璋冭瘯锛堢嚎 + 鍛戒腑鐐癸級 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Debug")
	bool bDrawGrabLineTraceDebug = false;

	/** 璋冭瘯缁樺埗鎸佺画鏃堕棿锛堢锛夛紱0 琛ㄧず浠呬竴甯?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Debug", meta=(ClampMin="0.0"))
	float GrabLineTraceDebugDrawTime = 1.0f;

	/** 璋冭瘯绾跨矖缁?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Debug", meta=(ClampMin="0.1"))
	float GrabLineTraceDebugThickness = 0.5f;

	// ==================== 鐩爣妫€娴嬬姸鎬?====================

	/** 褰撳墠鐬勫噯鐨勫彲鎶撳彇鐗╀綋 */
	UPROPERTY(BlueprintReadOnly, Category = "Grab")
	AActor* TargetedObject = nullptr;

	/** 褰撳墠鐬勫噯鐨勯楠煎悕锛堝鏋滅洰鏍囨槸楠ㄩ缃戞牸浣擄級 */
	UPROPERTY(BlueprintReadOnly, Category = "Grab")
	FName TargetedBoneName;
	UPROPERTY(BlueprintReadOnly, Category = "Grab")
	FVector TargetedImpactPoint = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "Grab")
	UPrimitiveComponent* TargetedHitComponent = nullptr;

	/** 灏勭嚎妫€娴嬫槸鍚﹀懡涓洰鏍?*/
	UPROPERTY(BlueprintReadOnly, Category = "Grab")
	bool bTraceHit = false;

	/** 褰撳墠瑙嗙嚎鏄惁鍙互鎵嬪姩鐐圭伀锛堢敤浜?UI 鎻愮ず锛?*/
	UPROPERTY(BlueprintReadOnly, Category = "Interact|Ignite")
	bool bCanIgniteBySight = false;

	/** 褰撳墠鐐圭伀鍙氦浜掔殑鍛戒腑鐐癸紙鐢ㄤ簬 UI 鎻愮ず锛?*/
	UPROPERTY(BlueprintReadOnly, Category = "Interact|Ignite")
	FVector IgniteBySightImpactPoint = FVector::ZeroVector;

	/** 瑙嗙嚎鍛戒腑缁勪欢鐢ㄤ簬鐐圭伀鍒ゅ畾鐨?Tag锛圕omponent Tag锛?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact|Ignite")
	FName IgniteBySightComponentTag = FName(TEXT("Interact_FireIgnite"));

	// ==================== 寮撶妯″紡閰嶇疆 ====================
	
	/** 鐬勫噯鏃跺乏鎵嬬殑浣嶇疆锛堢浉瀵逛簬鎽勫儚鏈猴級 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow|Aiming")
	FTransform AimingLeftHandTransform;

	/**
	 * PC 妯″紡鍥哄畾鎷夊紦璺濈锛堟部鎽勫儚鏈哄墠鍚戠殑鍙嶆柟鍚戞媺锛?
	 * 娉ㄦ剰锛氳繖鏄€滄墜鐨勪綅缃亸绉烩€濓紝涓嶆槸 Bow::MaxPullDistance銆?
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow|Draw")
	float PCDrawDistance = 50.0f;

	// ==================== 寮撶鐘舵€?====================
	
	/** 鏄惁姝ｅ湪鐬勫噯 */
	UPROPERTY(BlueprintReadOnly, Category = "Bow|State")
	bool bIsAiming = false;

	/** 鏄惁姝ｅ湪鎷夊紦 */
	UPROPERTY(BlueprintReadOnly, Category = "Bow|State")
	bool bIsDrawingBow = false;

	// ==================== 鎶曟幏锛圥C锛?====================

	/** 鏈€澶ф姇鎺峰皠绾胯窛绂伙紙鎽勫儚鏈哄墠鏂癸級 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw")
	float MaxThrowDistance = 1000.0f;

	/** 鎶曟幏鎶涚墿绾垮姬搴﹀弬鏁帮紙0-1锛岃秺灏忚秺骞筹級 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw", meta=(ClampMin="0.0", ClampMax="1.0"))
	float ThrowArcParam = 0.35f;

	// ==================== 瀹氳韩鏈紙PC锛?====================

	/** 瀹氳韩鐞冨彂灏勯€熷害鍊嶆暟锛堢浉瀵逛簬鎽勫儚鏈哄墠鍚戯級 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stasis")
	float StasisFireSpeedScalar = 1000.0f;

	/** 瀹氳韩鐞冪洰鏍囨娴嬪崐寰?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stasis")
	float StasisDetectionRadius = 1000.0f;

	/** 瀹氳韩鐞冪洰鏍囨娴嬫渶澶ц搴︼紙搴︼級 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stasis")
	float StasisDetectionAngle = 30.0f;
	
	
	// ==================== Movement ====================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float PCCrouchedHalfHeight = 40.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float PCMaxCrouchWalkSpeed = 200.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Crouch")
	float PCCrouchCameraInterpSpeed = 12.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Crouch", meta=(ClampMin="0.01"))
	float PCCrouchCameraStopThreshold = 0.5f;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetCrouched(bool bCrouch);
	/**
	 * 鎶曟幏鍏ュ彛锛堝敮涓€鍏ュ彛锛夈€?
	 * @param bRightHand true=鍙虫墜鎶曟幏锛宖alse=宸︽墜鎶曟幏銆?
	 */
	UFUNCTION(BlueprintCallable, Category = "Throw")
	void TryThrow(bool bRightHand);

	// ==================== 閲嶅啓鍩虹被 ====================
	
	/** 閲嶅啓锛氳繘鍏?閫€鍑哄紦绠ā寮?*/
	virtual void SetBowArmed(bool bArmed) override;

	// ==================== 杈撳叆澶勭悊 ======================================
	
	/**
	 * 澶勭悊宸︽壋鏈鸿緭鍏?
	 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void HandleLeftTrigger(bool bPressed);

	/**
	 * 澶勭悊鍙虫壋鏈鸿緭鍏?
	 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void HandleRightTrigger(bool bPressed);
	
	UFUNCTION(BlueprintCallable, Category = "Input")
	void HandleMoveInput(FVector2D MoveInput);
	
	UFUNCTION(BlueprintCallable, Category = "Input")
	void StartStarDraw();

	UFUNCTION(BlueprintCallable, Category = "Input")
	void StopStarDraw();


	/** PC 鎵嬪姩鐐圭伀锛堢敱钃濆浘杈撳叆浜嬩欢璋冪敤锛?*/
	UFUNCTION(BlueprintCallable, Category = "Input")
	void IgniteBySight();

	UFUNCTION(BlueprintCallable, Category = "Input")
	void TryWindowVaultBySight();

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetActionLocked(bool bLocked);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Input")
	bool IsActionLocked() const { return bActionLocked; }

	// ==================== 寮撶鎿嶄綔 ====================
	
	/**
	 * 寮€濮嬬瀯鍑嗭紙宸︽墜杩囨浮鍒扮瀯鍑嗕綅缃紝鍙栧嚭绠苟鐢卞彸鎵嬫姄浣忥級
	 */
	UFUNCTION(BlueprintCallable, Category = "Bow")
	void StartAiming();

	/**
	 * 鍋滄鐬勫噯
	 */
	UFUNCTION(BlueprintCallable, Category = "Bow")
	void StopAiming();

	/**
	 * ?????????????????????????????TryHandleStringHandEnter??????????
	 */
	UFUNCTION(BlueprintCallable, Category = "Bow")
	void StartDrawBow();

	/**
	 * 鍋滄鎷夊紦锛堝凡搴熷純锛氫竴鏃﹀紑濮嬫媺寮撲笉鑳藉彇娑堬紝浼氱洿鎺ュ彂灏勶級
	 */
	UFUNCTION(BlueprintCallable, Category = "Bow", meta = (DeprecatedFunction, DeprecationMessage = "StopDrawBow is deprecated. Once drawing starts, releasing will always fire the arrow."))
	void StopDrawBow();

	/**
	 * 閲婃斁寮撳鸡
	 */
	UFUNCTION(BlueprintCallable, Category = "Bow")
	void ReleaseBowString();
	void CancelDrawBow();

protected:
	// ==================== 鍐呴儴鍑芥暟 ====================

	/** 鏇存柊鐬勫噯鐩爣妫€娴嬶紙姣忓抚鎵ц锛?*/
	void UpdateTargetDetection();

	/** 鎵ц灏勭嚎妫€娴?*/
	bool PerformLineTrace(FHitResult& OutHit, float MaxDistance, ECollisionChannel TraceChannel) const;

	/** 澶勭悊 StasisPoint 鎶曟幏 */
	void HandleStasisPointThrow(UPCGrabHand* ThrowHand, class AStasisPoint* StasisPoint);

	/** 褰撴墜鎶撳彇鐗╀綋鏃剁殑鍥炶皟 */
	UFUNCTION()
	void OnHandGrabbedObject(AActor* GrabbedObject);

	AArrow* GetHeldRightHandArrow() const;
	bool EnsurePreparedArrowInRightHand();
	bool NockPreparedArrowFromRightHand();
	bool UnnockArrowToRightHand();
	void ReleasePCStringHoldWithoutFiring();
	void CleanupPreparedArrowWhenExitBowMode();
	void StoreAndDestroyArrow(AArrow* Arrow);


	/** 鎾斁鏃犵闊虫晥 */
	void PlayNoArrowSound();
	
	UPROPERTY(Transient)
	bool bIsCrouchCameraInterping = false;
	
	UPROPERTY(Transient)
	float RegularCameraRelativeZ = 0.0f;
	
	UPROPERTY(Transient)
	float RegularCapsuleHalfHeight = 0.0f;

	UPROPERTY(Transient)
	bool bActionLocked = false;
	
	void UpdateCrouchCameraInterp(float DeltaTime);
};
