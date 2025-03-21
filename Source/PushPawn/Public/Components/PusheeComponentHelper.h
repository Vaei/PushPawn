// Copyright (c) Jared Taylor. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "PusheeComponent.h"
#include "PushTypes.h"
#include "PusheeComponentHelper.generated.h"

/**
 * Blueprint implementation to allow blueprint-only devs to use PushPawn
 * Create a blueprint class that inherits from this class and implement the functions
 * Then, add the component to your pawn
 */
UCLASS(Blueprintable, ClassGroup=(Custom), meta=(NotBlueprintSpawnableComponent))
class PUSHPAWN_API UPusheeComponentHelper final
	: public UPusheeComponent
{
	GENERATED_BODY()

public:
	/**
	 * Prevents unnecessary ability activation
	 * Check conditions such as !IsPendingKillPending(), MovementMode != MOVE_None, IsAlive(), etc.
	 * @return True if we can be pushed by anyone at all 
	 */
	UFUNCTION(BlueprintPure, BlueprintImplementableEvent, Category=PushPawn, meta=(DisplayName="Is Pushable"))
	bool K2_IsPushable() const;
	virtual bool IsPushable() const override { return K2_IsPushable(); }

	/** @return True if we can currently be pushed by the PusherActor */
	UFUNCTION(BlueprintPure, BlueprintImplementableEvent, Category=PushPawn, meta=(DisplayName="Can Be Pushed By"))
	bool K2_CanBePushedBy(const AActor* PusherActor) const;
	virtual bool CanBePushedBy(const AActor* PusherActor) const override { return K2_CanBePushedBy(PusherActor); }

	/** @return Optional runtime strength scaling to change how much the pushee is pushed */
	UFUNCTION(BlueprintPure, BlueprintImplementableEvent, Category=PushPawn, meta=(DisplayName="Get Pushee Strength Scalar"))
	float K2_GetPusheeStrengthScalar() const;
	virtual float GetPusheeStrengthScalar() const override { return K2_GetPusheeStrengthScalar(); }
	
	/**
	 * Get the acceleration of the Pushee, will be normalized post-retrieval
	 * UCharacterMovementComponent::GetCurrentAcceleration()
	 */
	UFUNCTION(BlueprintPure, BlueprintImplementableEvent, Category=PushPawn, meta=(DisplayName="Get Pushee Acceleration"))
	FVector K2_GetPusheeAcceleration() const;
	virtual FVector GetPusheeAcceleration() const override { return K2_GetPusheeAcceleration(); }
	
	/**
	 * Get the velocity of the Pushee
	 * AActor::GetVelocity()
	 */
	UFUNCTION(BlueprintPure, BlueprintImplementableEvent, Category=PushPawn, meta=(DisplayName="Get Pushee Velocity"))
	FVector K2_GetPusheeVelocity() const;
	virtual FVector GetPusheeVelocity() const override { return K2_GetPusheeVelocity(); }

	/**
	 * Whether the pushee is moving on the ground (not in the air)
	 * UPawnMovementComponent::IsMovingOnGround()
	 */
	UFUNCTION(BlueprintPure, BlueprintImplementableEvent, Category=PushPawn, meta=(DisplayName="Is Pushee Moving On Ground"))
	bool K2_IsPusheeMovingOnGround() const;
	virtual bool IsPusheeMovingOnGround() const override { return K2_IsPusheeMovingOnGround(); }

	/**
	 * Get the collision shape of the pushee.
	 * 
	 * If the size changes during runtime, it can de-sync -- i.e. use the default shape size.
	 * This means crouch, prone, etc. consideration is not supported by default. It may be possible to extend this.
	 */
	UFUNCTION(BlueprintPure, BlueprintImplementableEvent, Category=PushPawn, meta=(DisplayName="Get Pushee Collision Shape"))
	FPushPawnCollisionShapeHelper K2_GetPusheeCollisionShape(FQuat& ShapeRotation) const;
	virtual FCollisionShape GetPusheeCollisionShape(FQuat& ShapeRotation) const override { return K2_GetPusheeCollisionShape(ShapeRotation).ToCollisionShape(); }

protected:
	/** 
	 * @return The default collision shape for the pushee
	 * @param Actor				The actor to get the collision shape for
	 * @param ShapeRotation		Output parameter for the shape's rotation
	 * @param OptionalShapeType	The optional shape type to use
	 * @param OptionalComponent	The optional component to use - if not supplied, the root component from the Actor's defaults will be used
	 */
	UFUNCTION(BlueprintPure, Category=PushPawn, meta=(DisplayName="Get Default Pushee Collision Shape"))
	FPushPawnCollisionShapeHelper K2_GetDefaultPusheeCollisionShape(const AActor* Actor, FQuat& ShapeRotation, EPushCollisionType OptionalShapeType = EPushCollisionType::None, USceneComponent* OptionalComponent = nullptr) const;
};
