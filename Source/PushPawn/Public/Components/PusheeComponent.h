// Copyright (c) Jared Taylor. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "IPush.h"
#include "PushPawnComponent.h"
#include "CollisionShape.h"
#include "PusheeComponent.generated.h"


/**
 * Optional component for actors that can be pushed
 * Allows implementing logic on the component instead of the actor
 */
UCLASS(Abstract)
class PUSHPAWN_API UPusheeComponent
	: public UPushPawnComponent
	, public IPusheeInstigator
{
	GENERATED_BODY()

public:
	/**
	 * Prevents unnecessary ability activation
	 * Check conditions such as !IsPendingKillPending(), MovementMode != MOVE_None, IsAlive(), etc.
	 * @return True if we can be pushed by anyone at all 
	 */
	virtual bool IsPushable() const override PURE_VIRTUAL(UPusheeComponent::IsPushable, return false;);

	/** @return True if we can currently be pushed by the PusherActor */
	virtual bool CanBePushedBy(const AActor* PusherActor) const override PURE_VIRTUAL(UPusheeComponent::CanBePushedBy, return false;);

	/** @return Optional runtime strength scaling to change how much the pushee is pushed */
	virtual float GetPusheeStrengthScalar() const override { return 1.f; }
	
	/**
	 * Get the acceleration of the Pushee, will be normalized post-retrieval
	 * UCharacterMovementComponent::GetCurrentAcceleration()
	 */
	virtual FVector GetPusheeAcceleration() const override { return FVector::ZeroVector; }
	
	/**
	 * Get the velocity of the Pushee
	 * AActor::GetVelocity()
	 */
	virtual FVector GetPusheeVelocity() const override { return FVector::ZeroVector; }

	/**
	 * Whether the pushee is moving on the ground (not in the air)
	 * UPawnMovementComponent::IsMovingOnGround()
	 */
	virtual bool IsPusheeMovingOnGround() const override PURE_VIRTUAL(UPusheeComponent::IsPusheeMovingOnGround, return true;);

	/**
	 * Get the collision shape of the pushee.
	 * 
	 * If the size changes during runtime, it can de-sync -- i.e. use the default shape size.
	 * This means crouch, prone, etc. consideration is not supported by default. It may be possible to extend this.
	 * 
	 * Generally you should use UPushStatics::GetDefaultPusheeCollisionShape() to get the shape, unless you want to use a
	 * different shape size than the collision shape's.
	 */
	virtual FCollisionShape GetPusheeCollisionShape(FQuat& ShapeRotation) const override PURE_VIRTUAL(UPusheeComponent::GetPusheeCollisionShape, return {};);

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
	virtual FOnPushPawnScanPaused* GetPushPawnScanPausedDelegate() override { return nullptr; }
};
