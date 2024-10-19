// Copyright (c) Jared Taylor. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "PushTypes.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_GrantNearbyPush.generated.h"

/**
 * Used by the PushPawn Scan Ability to grant Push abilities to nearby pushees.
 */
UCLASS()
class PUSHPAWN_API UAbilityTask_GrantNearbyPush final : public UAbilityTask
{
	GENERATED_BODY()

private:
	UPROPERTY()
	FPushPawnScanParams ScanParams;

	UPROPERTY()
	float BaseScanRange = 100.f;
	
	FTimerHandle TimerHandle;

	TMap<FObjectKey, FGameplayAbilitySpecHandle> PushAbilityCache;
	
public:
	virtual bool IsSupportedForNetworking() const override { return true; }

	virtual void Activate() override;

private:
	virtual void OnDestroy(bool bInOwnerFinished) override;

	void ActivateTimer();

	void QueryPushes();

public:
	/** Wait until an overlap occurs. */
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_GrantNearbyPush* GrantAbilitiesForNearbyPushers(UGameplayAbility* OwningAbility, const FPushPawnScanParams& InScanParams, float InBaseScanRange);
};
