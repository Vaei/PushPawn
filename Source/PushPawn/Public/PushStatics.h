// Copyright (c) Jared Taylor. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "PushTypes.h"
#include "PushStatics.generated.h"

class IPusheeInstigator;
struct FGameplayEventData;
struct FGameplayAbilityTargetDataHandle;
struct FPushPawnAbilityTargetData;
class IPusherTarget;

/**
 * Static helper functions for PushPawn
 */
UCLASS()
class PUSHPAWN_API UPushStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
protected:
	//--------------------------------------------------------------
	// EVENT DATA EXTRACTION METHODS
	//--------------------------------------------------------------

	static void GetPushActorsFromEventData(const FGameplayEventData& EventData, const AActor*& Pushee, const AActor*& Pusher);
	
public:
	/** Extract Pawns from EventData */
	template <typename PusheeT, typename PusherT>
	static void GetPushPawnsFromEventData(const FGameplayEventData& EventData, const PusheeT*& Pushee, const PusherT*& Pusher);

	/** Extract Pawns from EventData using CastChecked - will crash if type is incorrect */
	template <typename PusheeT, typename PusherT>
	static void GetPushPawnsFromEventDataChecked(const FGameplayEventData& EventData, const PusheeT*& Pushee, const PusherT*& Pusher);

	/** Extract Pusher Pawn from EventData */
	UFUNCTION(BlueprintCallable, Category=PushPawn, meta=(DisplayName="Get Pusher Pawn From Event Data", DeterminesOutputType="PawnClass", DynamicOutputParam="Pusher"))
	static void K2_GetPusherPawnFromEventData(AActor*& Pusher, const FGameplayEventData& EventData, TSubclassOf<APawn> PawnClass);

	/** Extract Pushee Pawn from EventData */
	UFUNCTION(BlueprintCallable, Category=PushPawn, meta=(DisplayName="Get Pushee Pawn From Event Data", DeterminesOutputType="PawnClass", DynamicOutputParam="Pushee"))
	static void K2_GetPusheePawnFromEventData(AActor*& Pushee, const FGameplayEventData& EventData, TSubclassOf<APawn> PawnClass);
	
	/**
	 * Extracts the push direction from the event data and normalizes it
	 * @param EventData		The event data to extract the push direction from
	 * @param bForce2D		Whether to force the direction to be 2D
	 * @return The normalized push direction from the event data
	 */
	UFUNCTION(BlueprintCallable, Category=PushPawn)
	static FVector GetPushDirectionFromEventData(const FGameplayEventData& EventData, bool bForce2D = true);

public:
	//--------------------------------------------------------------
	// ABILITY HELPER METHODS
	//--------------------------------------------------------------

	/** @return True if the pawn is moving on the ground */
	UFUNCTION(BlueprintPure, Category=PushPawn)
	static bool IsPawnMovingOnGround(const APawn* Pawn);
	
	static bool IsPusheeMovingOnGround(const IPusheeInstigator* Pushee);

	/** @return The velocity of the pawn factoring incline if on ground */
	UFUNCTION(BlueprintPure, Category=PushPawn)
	static FVector GetPawnGroundVelocity(const APawn* Pawn);
	
	static FVector GetPusheeGroundVelocity(const IPusheeInstigator* Pushee);

	/** @return The speed of the pawn factoring incline if on ground */
	UFUNCTION(BlueprintPure, Category=PushPawn)
	static float GetPawnGroundSpeed(const APawn* Pawn);
	
	static float GetPusheeGroundSpeed(const IPusheeInstigator* Pushee);

public:
	/** 
	 * Returns the push strength based on the push parameters
	 * @param Pushee	The pawn being pushed
	 * @param Params	The push parameters
	 * @return The push strength
	 */
	UFUNCTION(BlueprintPure, Category=PushPawn)
	static float GetPushStrength(const APawn* Pushee, const FPushPawnActionParams& Params);

	/** 
	 * Returns the push strength based on the push parameters
	 * @param Pushee					The pawn being pushed
	 * @param VelocityToStrengthCurve	The curve mapping velocity to push strength
	 * @param StrengthScalar			The scalar to apply to the push strength. If no curve supplied, this will be the direct push strength
	 * @return The push strength
	 */
	UFUNCTION(BlueprintPure, Category=PushPawn)
	static float GetPushStrengthSimple(const APawn* Pushee, const UCurveFloat* VelocityToStrengthCurve = nullptr, float StrengthScalar = 1.f);

public:
	/** 
	 * Returns degree of the angle between Direction and Rotation forward vector
	 * The range of return will be from [-180, 180].
	 * @param	Direction		The direction relative to BaseRotation
	 * @param	BaseRotation	The base rotation, e.g. of a pawn
	 */
	static float CalculatePushDirection(const FVector& Direction, const FRotator& BaseRotation);

	UFUNCTION(BlueprintCallable, Category=PushPawn, meta=(ExpandEnumAsExecs="ValidPushDirection"))
	static EPushCardinal GetPushDirection(const AActor* FromActor, const AActor* ToActor, EValidPushDirection& ValidPushDirection);

public:
	//--------------------------------------------------------------
	// INTERNAL PUSH PAWN HELPER METHODS
	//--------------------------------------------------------------

	static FVector GetPushPawnAcceleration(const IPusheeInstigator* Pushee);
	static FVector GetPushPawnAcceleration(APawn* Pushee);
	static bool IsPusheeAccelerating(const IPusheeInstigator* Pushee);
	static bool IsPusheeAccelerating(APawn* Pushee);
	static bool IsPusheeAccelerating(const FVector& Acceleration);
	
	static const float& GetPushPawnScanRate(const IPusheeInstigator* Pushee, const FPushPawnScanParams& ScanParams);
	static const float& GetPushPawnScanRate(APawn* Pushee, const FPushPawnScanParams& ScanParams);
	static const float& GetPushPawnScanRate(const FVector& Acceleration, const FPushPawnScanParams& ScanParams);

	static float GetPushPawnScanRange(const IPusheeInstigator* Pushee, float BaseScanRange, const FPushPawnScanParams& ScanParams);
	static float GetPushPawnScanRange(APawn* Pushee, float BaseScanRange, const FPushPawnScanParams& ScanParams);
	static float GetPushPawnScanRange(const FVector& Acceleration, float BaseScanRange, const FPushPawnScanParams& ScanParams);

	static EPushCollisionType GetPusheeCollisionShapeType(const AActor* Actor);

	/** 
	 * @return The default collision shape for the pushee
	 * @param Actor				The actor to get the collision shape for
	 * @param OptionalShapeType	The optional shape type to use
	 * @param OptionalComponent	The optional component to use - if not supplied, the root component from the Actor's defaults will be used
	 */
	static FCollisionShape GetDefaultPusheeCollisionShape(const AActor* Actor, EPushCollisionType OptionalShapeType = EPushCollisionType::None, USceneComponent* OptionalComponent = nullptr);
	
	/**
	 * @return The max of the collision shape size. For ACharacter: ScaledCapsuleHalfHeight or ScaledCapsuleRadius - whichever is larger - taken from class defaults (i.e. ignores crouching character)
	 * @see ActivateAbility - this does not update if changed after ActivateAbility
	 */
	UFUNCTION(BlueprintPure, Category=PushPawn)
	static float GetMaxDefaultCollisionShapeSize(const AActor* ActorWithSuitableRootComponent, EPushCollisionType SpecificShapeType = EPushCollisionType::None);
	
	/**
	 * Retrieves the AActor associated with the given PushTarget
	 * @param PushTarget The target interface from which to extract the actor.
	 * @return The AActor if the PushTarget is an actor or a component of an actor, otherwise nullptr.
	 */
	UFUNCTION(BlueprintCallable, Category=PushPawn)
	static AActor* GetActorFromPushTarget(TScriptInterface<IPusherTarget> PushTarget);

	/**
	 * Retrieves all components and the actor itself that implement the IPusherTarget interface from the given actor.
	 * @param Actor				The actor to search for push targets.
	 * @param OutPushTargets	The array to populate with found push targets.
	 */
	UFUNCTION(BlueprintCallable, Category=PushPawn)
	static void GetPushTargetsFromActor(AActor* Actor, TArray<TScriptInterface<IPusherTarget>>& OutPushTargets);

public:
	/** 
	 * Appends push targets from the given overlap results to the output array.
	 * @param OverlapResults The array of overlap results to process.
	 * @param OutPushTargets The array to populate with found push targets.
	 */
	static void AppendPushTargetsFromOverlapResults(const TArray<FOverlapResult>& OverlapResults, TArray<TScriptInterface<IPusherTarget>>& OutPushTargets);

	/**
	 * Appends push targets from the given hit result to the output array.
	 * @param HitResult The hit result to process.
	 * @param OutPushTargets The array to populate with found push targets.
	 */
	static void AppendPushTargetsFromHitResult(const FHitResult& HitResult, TArray<TScriptInterface<IPusherTarget>>& OutPushTargets);
};

template <typename PusheeT, typename PusherT>
void UPushStatics::GetPushPawnsFromEventData(const FGameplayEventData& EventData, const PusheeT*& Pushee, const PusherT*& Pusher)
{
	const AActor* PusheeActor = nullptr;
	const AActor* PusherActor = nullptr;
	GetPushActorsFromEventData(EventData, PusheeActor, PusherActor);

	Pushee = Cast<PusheeT>(PusheeActor);
	Pusher = Cast<PusherT>(PusherActor);
}

template <typename PusheeT, typename PusherT>
void UPushStatics::GetPushPawnsFromEventDataChecked(const FGameplayEventData& EventData, const PusheeT*& Pushee, const PusherT*& Pusher)
{
	const AActor* PusheeActor = nullptr;
	const AActor* PusherActor = nullptr;
	GetPushActorsFromEventData(EventData, PusheeActor, PusherActor);

	Pushee = CastChecked<PusheeT>(PusheeActor);
	Pusher = CastChecked<PusherT>(PusherActor);
}
