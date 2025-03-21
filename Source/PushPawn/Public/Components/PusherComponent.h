// Copyright (c) Jared Taylor. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "IPush.h"
#include "PushPawnComponent.h"
#include "PusherComponent.generated.h"


UCLASS(Abstract)
class PUSHPAWN_API UPusherComponent
	: public UPushPawnComponent
	, public IPusherTarget
{
	GENERATED_BODY()

public:
	/** Fills OptionBuilder with FPushOption, which contains data such as the push ability and useful vectors */
	virtual void GatherPushOptions(const FPushQuery& PushQuery, FPushOptionBuilder& OptionBuilder) override PURE_VIRTUAL(UPusherComponent::GatherPushOptions, ;);

	/** Optionally, allow the Pusher to manipulate the event data */
	virtual void CustomizePushEventData(const FGameplayTag& PushEventTag, FGameplayEventData& InOutEventData) override { }

	/**
	 * Prevents unnecessary ability activation
	 * Check conditions such as !IsPendingKillPending(), MovementMode != MOVE_None, IsAlive(), etc.
	 * @return True if we can push anyone at all 
	 */
	virtual bool IsPushCapable() const override PURE_VIRTUAL(UPusheeComponent::IsPushCapable, return false;);

	/** @return True if we can currently push the PusheeActor */
	virtual bool CanPushPawn(const AActor* PusheeActor) const override PURE_VIRTUAL(UPusheeComponent::CanPushPawn, return false;);

	/** @return Optional runtime strength scaling to change how much the pusher pushes the pushee */
	virtual float GetPusherStrengthScalar() const override { return 1.f; }
};
