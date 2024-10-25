// Copyright (c) Jared Taylor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PusherComponent.h"
#include "PusherComponentHelper.generated.h"


/**
 * Blueprint implementation to allow blueprint-only devs to use PushPawn
 * Create a blueprint class that inherits from this class and implement the functions
 * Then, add the component to your pawn
 */
UCLASS(Blueprintable, ClassGroup=(Custom), meta=(NotBlueprintSpawnableComponent))
class PUSHPAWN_API UPusherComponentHelper final
	: public UPusherComponent
{
	GENERATED_BODY()

public:
	/** Get the ability that defines how we push another pawn */
	UFUNCTION(BlueprintPure, BlueprintImplementableEvent, Category=PushPawn, meta=(DisplayName="Get Push Ability"))
	TSubclassOf<UGameplayAbility> K2_GetPushAbility();
	virtual void GatherPushOptions(const FPushQuery& PushQuery, FPushOptionBuilder& OptionBuilder) override;

	/** Optionally, allow the Pusher to manipulate the event data - Not available to BP */
	virtual void CustomizePushEventData(const FGameplayTag& PushEventTag, FGameplayEventData& InOutEventData) override { }

	/**
	 * Prevents unnecessary ability activation
	 * Check conditions such as !IsPendingKillPending(), MovementMode != MOVE_None, IsAlive(), etc.
	 * @return True if we can push anyone at all 
	 */
	UFUNCTION(BlueprintPure, BlueprintImplementableEvent, Category=PushPawn, meta=(DisplayName="Is Push Capable"))
	bool K2_IsPushCapable() const;
	virtual bool IsPushCapable() const override { return K2_IsPushCapable(); }

	/** @return True if we can currently push the PusheeActor */
	UFUNCTION(BlueprintPure, BlueprintImplementableEvent, Category=PushPawn, meta=(DisplayName="CanPushPawn"))
	bool K2_CanPushPawn(const AActor* PusheeActor) const;
	virtual bool CanPushPawn(const AActor* PusheeActor) const override { return K2_CanPushPawn(PusheeActor); }
};
