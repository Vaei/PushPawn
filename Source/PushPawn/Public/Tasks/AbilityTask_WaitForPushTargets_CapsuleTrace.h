// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilityTask_WaitForPushTargets.h"
#include "AbilityTask_WaitForPushTargets_CapsuleTrace.generated.h"

class AActor;
class UPrimitiveComponent;

UCLASS()
class PUSHPAWN_API UAbilityTask_WaitForPushTargets_CapsuleTrace : public UAbilityTask_WaitForPushTargets
{
	GENERATED_UCLASS_BODY()

	virtual void Activate() override;

	/** Wait until we trace new set of Pushs.  This task automatically loops. */
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_WaitForPushTargets_CapsuleTrace* WaitForPushTargets_CapsuleTrace(UGameplayAbility* OwningAbility, FPushQuery PushQuery, ECollisionChannel TraceChannel, FGameplayAbilityTargetingLocationInfo StartLocation, float PushScanRange = 100, float PushScanRate = 0.100);

private:

	virtual void OnDestroy(bool AbilityEnded) override;

	void PerformTrace();

	UPROPERTY()
	FPushQuery PushQuery;

	UPROPERTY()
	FGameplayAbilityTargetingLocationInfo StartLocation;

	float PushScanRange = 100;
	float PushScanRate = 0.100;

	FTimerHandle TimerHandle;
};
