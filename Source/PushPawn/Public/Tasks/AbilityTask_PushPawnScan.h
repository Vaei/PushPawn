// Copyright (c) Jared Taylor. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AbilityTask_PushPawnScan_Base.h"
#include "PushTypes.h"
#include "AbilityTask_PushPawnScan.generated.h"

class UAbilityTask_PushPawnSync;

/**
 * Implemented by the PushPawn Scan ability.
 * 
 * This task loops automatically until the ability is deactivated.
 * 
 * This task intermittently waits for a net sync to occur to prevent server/client de-sync.
 *
 * Activation failures are handled by a delay, thus so long as the conditions are eventually met it will activate.
 */
UCLASS()
class PUSHPAWN_API UAbilityTask_PushPawnScan : public UAbilityTask_PushPawnScan_Base
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void OnNetSync(UAbilityTask_PushPawnSync* SyncPoint);
	
	void ActivateTimer(EPushPawnPauseType PauseType = EPushPawnPauseType::NotPaused);

	virtual void Activate() override;

	/**
	 * Wait until we trace new set of Pushes. This task automatically loops.
	 * @param OwningAbility The ability that owns this task
	 * @param PushQuery The query to use for the trace
	 * @param StartLocation The location to start the trace from
	 * @param ScanParams The parameters to use for the scan
	 * @param ActivationFailureDelay The delay to wait before activating the ability again if the scan fails to activate due to invalid data
	 */
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_PushPawnScan* PushPawnScan(
		UGameplayAbility* OwningAbility, FPushQuery PushQuery,
		FGameplayAbilityTargetingLocationInfo StartLocation, const FPushPawnScanParams& ScanParams,
		float ActivationFailureDelay = 0.2f);

private:
	virtual void OnDestroy(bool bInOwnerFinished) override;

	void PerformTrace();

	void OnScanPaused(bool bIsPaused);
	
	UPROPERTY(Transient, DuplicateTransient)
	FPushPawnScanParams ScanParams;

	UPROPERTY(Transient, DuplicateTransient)
	FGameplayAbilityTargetingLocationInfo StartLocation;
	
	UPROPERTY(Transient, DuplicateTransient)
	FPushQuery PushQuery;

	UPROPERTY(Transient, DuplicateTransient)
	float ActivationFailureDelay;

	/** Tracked to prevent premature GC and allow ending during OnDestroy */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UAbilityTask_PushPawnSync>> SyncPoints;
	
private:
	UPROPERTY(Transient, DuplicateTransient)
	float CurrentScanRate;

	FTimerHandle TimerHandle;

	FConsoleVariableDelegate OnDisabledDelegate;
	
	FOnPushPawnScanPaused* OnPushPawnScanPauseStateChangedDelegate = nullptr;
};
