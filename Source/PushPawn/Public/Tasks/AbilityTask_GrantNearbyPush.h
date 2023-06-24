// Copyright (c) Jared Taylor. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_GrantNearbyPush.generated.h"

/**
 * 
 */
UCLASS()
class PUSHPAWN_API UAbilityTask_GrantNearbyPush final : public UAbilityTask
{
	GENERATED_BODY()

private:
	ECollisionChannel TraceChannel;
	
	float PushScanRange = 100;
	float PushScanRate = 0.100;

	FTimerHandle QueryTimerHandle;

	TMap<FObjectKey, FGameplayAbilitySpecHandle> PushAbilityCache;
	
public:
	virtual bool IsSupportedForNetworking() const override { return true; }

	virtual void Activate() override;

private:
	virtual void OnDestroy(bool bInOwnerFinished) override;

	void QueryPushs();

public:
	/** Wait until an overlap occurs. */
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_GrantNearbyPush* GrantAbilitiesForNearbyPushers(UGameplayAbility* OwningAbility, ECollisionChannel TraceChannel, float PushScanRange, float PushScanRate);
};
