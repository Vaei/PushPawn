// Copyright (c) Jared Taylor. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "PushOption.h"
#include "IPush.generated.h"

class IPusherTarget;
struct FPushQuery;

DECLARE_DELEGATE_OneParam(FOnPushPawnScanPaused, bool /* bIsPaused */);

/**
 * Builder class for PushOptions
 * This is used to fill an array of PushOptions with the necessary data
 */
class FPushOptionBuilder
{
public:
	FPushOptionBuilder(TScriptInterface<IPusherTarget> InterfaceTargetScope, TArray<FPushOption>& PushOptions)
		: Scope(InterfaceTargetScope)
		, Options(PushOptions)
	{
	}

	/** Add a PushOption to the array */
	void AddPushOption(const FPushOption& Option) const
	{
		FPushOption& OptionEntry = Options.Add_GetRef(Option);
		OptionEntry.PusherTarget = Scope;
	}

private:
	/** The interface of the target that is being pushed */
	TScriptInterface<IPusherTarget> Scope;

	/** The array of PushOptions to fill */
	TArray<FPushOption>& Options;
};

/** Interface for the instigator who gets pushed */
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UPusheeInstigator : public UInterface
{
	GENERATED_BODY()
};

/** Interface for the instigator who gets pushed */
class IPusheeInstigator
{
	GENERATED_BODY()

public:
	/**
	 * Prevents unnecessary ability activation
	 * Check conditions such as !IsPendingKillPending(), MovementMode != MOVE_None, IsAlive(), etc.
	 * @return True if we can be pushed by anyone at all 
	 */
	virtual bool IsPushable() const = 0;

	/** @return True if we can currently be pushed by the PusherActor */
	virtual bool CanBePushedBy(const AActor* PusherActor) const = 0;

	/** @return Optional runtime strength scaling to change how much the pushee is pushed */
	virtual float GetPusheeStrengthScalar() const { return 1.f; }

	/**
	 * @param Strength The strength to push the pushee
	 * @return True if Strength should override the applied strength
	 */
	virtual bool GetPusheeStrengthOverride(float& Strength) const { return false; }

	/**
	 * Get the acceleration of the Pushee, will be normalized post-retrieval
	 * UCharacterMovementComponent::GetCurrentAcceleration()
	 */
	virtual FVector GetPusheeAcceleration() const { return FVector::ZeroVector; }
	
	/**
	 * Get the velocity of the Pushee
	 * AActor::GetVelocity()
	 */
	virtual FVector GetPusheeVelocity() const { return FVector::ZeroVector; }

	/**
	 * Whether the pushee is moving on the ground (not in the air)
	 * UPawnMovementComponent::IsMovingOnGround()
	 */
	virtual bool IsPusheeMovingOnGround() const = 0;

	/**
	 * Get the collision shape of the pushee.
	 * 
	 * If the size changes during runtime, it can de-sync -- i.e. use the default shape size.
	 * This means crouch, prone, etc. consideration is not supported by default. It may be possible to extend this.
	 * 
	 * Generally you should use UPushStatics::GetDefaultPusheeCollisionShape() to get the shape, unless you want to use a
	 * different shape size than the collision shape's.
	 */
	virtual FCollisionShape GetPusheeCollisionShape(FQuat& ShapeRotation) const = 0;

	/**
	 * Optionally, pause the scan when the pawn is in a state where it should not look for pushers.
	 * 
	 * This is useful for optimizing performance, consider integrating with AI significance, spatial hashing,
	 * proximity checks, rep graph relevance, etc.
	 * 
	 * To implement, add FOnPushPawnScanPauseStateChanged to your Pawn and return it here
	 * Then, when the scan should be paused or resumed, call the delegate
	 * 
	 * @return Delegate for when the scan is paused or resumed
	 */
	virtual FOnPushPawnScanPaused* GetPushPawnScanPausedDelegate() { return nullptr; }
};

/** Interface for the target who does the pushing */
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UPusherTarget : public UInterface
{
	GENERATED_BODY()
};

/** Interface for the target who does the pushing */
class IPusherTarget
{
	GENERATED_BODY()

public:
	/** Fills OptionBuilder with FPushOption, which contains data such as the push ability and useful vectors */
	virtual void GatherPushOptions(const FPushQuery& PushQuery, FPushOptionBuilder& OptionBuilder) = 0;

	/** Optionally, allow the Pusher to manipulate the event data */
	virtual void CustomizePushEventData(const FGameplayTag& PushEventTag, FGameplayEventData& InOutEventData) { }

	/**
	 * Prevents unnecessary ability activation
	 * Check conditions such as !IsPendingKillPending(), MovementMode != MOVE_None, IsAlive(), etc.
	 * @return True if we can push anyone at all 
	 */
	virtual bool IsPushCapable() const = 0;

	/** @return True if we can currently push the PusheeActor */
	virtual bool CanPushPawn(const AActor* PusheeActor) const = 0;

	/** @return Optional runtime strength scaling to change how much the pusher pushes the pushee */
	virtual float GetPusherStrengthScalar() const { return 1.f; }

	/**
	 * @param Strength The strength to push the pushee
	 * @return True if Strength should override the applied strength
	 */
	virtual bool GetPusherStrengthOverride(float& Strength) const { return false; }
};