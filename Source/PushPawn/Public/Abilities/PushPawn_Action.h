// Copyright (c) Jared Taylor. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "PushPawn_Action_Base.h"
#include "PushTypes.h"
#include "PushPawn_Action.generated.h"

/**
 * A completed C++ implementation of the PushPawn_Action_Base class
 * This is significantly cheaper performance-wise than using Blueprint
 *
 * BP should only be used for changing parameters, not for logic.
 */
UCLASS(Blueprintable)
class PUSHPAWN_API UPushPawn_Action // final -- Make sure your subclass is marked final, do not have non-final classes for this ability
	: public UPushPawn_Action_Base
{
	GENERATED_BODY()

protected:
	/** The parameters to apply to the pushee */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=PushPawn)
	FPushPawnActionParams PushParams;

	virtual bool ActivatePushPawnAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UFUNCTION()
	virtual void OnPushTaskFinished();
};
