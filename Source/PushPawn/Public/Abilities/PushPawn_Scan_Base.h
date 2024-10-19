// Copyright (c) Jared Taylor. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "PushOption.h"
#include "PushPawn_Ability.h"
#include "PushTypes.h"
#include "PushPawn_Scan_Base.generated.h"

/**
 * The base class for all PushPawn scanning
 * This is a lightweight class that cannot use tags or other advanced features to reduce performance overhead
 *
 * This checks for pawns within a certain range, and then triggers a push based on the options available.
 */
UCLASS(Blueprintable)
class PUSHPAWN_API UPushPawn_Scan_Base : public UPushPawn_Ability
{
	GENERATED_BODY()

protected:
	/** Adjust the behavior of the pawn when scanning for other pawns */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=PushPawn)
	FPushPawnScanParams ScanParams;

	/** Information needed to push a pawn. */
	UPROPERTY(BlueprintReadWrite, Category=PushPawn)
	TArray<FPushOption> CurrentOptions;

	/** The range to scan for pawns, later scaled by FPushPawnScanParams */
	UPROPERTY(BlueprintReadOnly, Category=PushPawn)
	float BaseScanRange;

	/**
	 * The number of pushes that can be triggered before a net sync is required
	 * Will not sync until MinNetSyncDelay has also passed
	 * Set to 0 to disable
	 * @see ShouldWaitForNetSync(), TriggeredPushesSinceLastNetSync, MinNetSyncDelay
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=PushPawn, meta=(ClampMin="0", UIMin="0"))
	int32 MaxPushesUntilNetSync = 6;

	/**
	 * Don't allow net sync to occur until this time has passed since the last push
	 * Set to 0 to disable
	 * @see ShouldWaitForNetSync(), LastPushTime
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=PushPawn, meta=(ClampMin="0.0", UIMin="0.0"))
	float MinNetSyncDelay = 1.0f;
	
	/**
	 * If this time lapses after any push, a net sync will occur
	 * Set to 0 to disable
	 * @see ShouldWaitForNetSync(), LastPushTime
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=PushPawn, meta=(ClampMin="0.0", UIMin="0.0"))
	float NetSyncDelayAfterPush = 5.0f;

protected:
	/**
	 * The number of pushes that have been triggered since the last net sync
	 * @see ShouldWaitForNetSync(), MaxPushesUntilNetSync
	 */
	UPROPERTY(BlueprintReadOnly, Category=PushPawn)
	int32 TriggeredPushesSinceLastNetSync = 0;

	UPROPERTY(BlueprintReadOnly, Category=PushPawn)
	float LastPushTime = 0.f;

public:
	UPushPawn_Scan_Base(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void OnGameplayTaskInitialized(UGameplayTask& Task) override;
	
protected:
	virtual void ActivatePushPawnAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UFUNCTION(BlueprintCallable, Category=PushPawn)
	void UpdatePushes(const TArray<FPushOption>& PushOptions);

	UFUNCTION(BlueprintCallable, Category=PushPawn)
	void TriggerPush();

protected:
	/**
	 * Get the base scan range for the pawn
	 * This is the largest size (halfheight vs radius) of our capsule
	 * Only used if bAllowBlueprintImplementation is FALSE
	 * 
	 * @see UPushStatics::GetMaxCapsuleSize() - default return value
	 */
	virtual float GetBaseScanRange(const AActor* AvatarActor) const;
	
	/**
	 * Get the base scan range for the pawn
	 * This is the largest size (halfheight vs radius) of our capsule
	 * Only used if bAllowBlueprintImplementation is TRUE
	 * 
	 * @see UPushStatics::GetMaxCapsuleSize() - default return value
	 */
	UFUNCTION(BlueprintNativeEvent, Category=PushPawn, meta=(DisplayName="Get Base Scan Range"))
	float K2_GetBaseScanRange(const AActor* AvatarActor) const;

public:
	/**
	 * Eventually PushPawn gets out of sync, so we need to send a prediction key to the server to resync
	 * Without this, over time it will start to de-sync
	 * @see TriggeredPushesSinceLastNetSync, MaxPushesUntilNetSync
	 */
	virtual bool ShouldWaitForNetSync() const;

	/** Reset the number of pushes since the last net sync */
	virtual void ConsumeWaitForNetSync();
};
