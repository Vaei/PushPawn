// Copyright (c) Jared Taylor. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "PushPawn_Scan_Base.h"
#include "PushPawn_Scan.generated.h"

/**
 * A completed C++ implementation of the PushPawn_Scan_Core class
 * This is significantly cheaper performance-wise than using Blueprint
 *
 * BP should only be used for changing parameters, not for logic.
 */
UCLASS(Blueprintable)
class PUSHPAWN_API UPushPawn_Scan final : public UPushPawn_Scan_Base
{
	GENERATED_BODY()

public:
	virtual void ActivatePushPawnAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UFUNCTION()
	void OnPushObjectsChanged(const TArray<FPushOption>& PushOptions);
};
