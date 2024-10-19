// Copyright (c) Jared Taylor. All Rights Reserved

#include "PushStatics.h"
#include "IPush.h"
#include "Abilities/PushPawnAbilityTargetData.h"
#include "Components/CapsuleComponent.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/PawnMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PushStatics)

void UPushStatics::GetPushActorsFromEventData(const FGameplayEventData& EventData, const AActor*& Pushee, const AActor*& Pusher)
{
	Pushee = EventData.Instigator.Get();
	Pusher = EventData.Target.Get();
}

void UPushStatics::K2_GetPusherPawnFromEventData(AActor*& Pusher, const FGameplayEventData& EventData,
	TSubclassOf<APawn> PawnClass)
{
	// Pusher is the target
	if (PawnClass)
	{
		Pusher = const_cast<AActor*>(EventData.Target.Get());
	}
	else
	{
		Pusher = nullptr;
	}
}

void UPushStatics::K2_GetPusheePawnFromEventData(AActor*& Pushee, const FGameplayEventData& EventData,
	TSubclassOf<APawn> PawnClass)
{
	// Pushee is the instigator
	if (PawnClass)
	{
		Pushee = const_cast<AActor*>(EventData.Instigator.Get());
	}
	else
	{
		Pushee = nullptr;
	}
}

FVector UPushStatics::GetPushDirectionFromEventData(const FGameplayEventData& EventData, bool bForce2D)
{
	// Get the target data from the event data
	const FGameplayAbilityTargetData* RawData = EventData.TargetData.Get(0);
	check(RawData);
	const FPushPawnAbilityTargetData& PushTargetData = static_cast<const FPushPawnAbilityTargetData&>(*RawData);

	// Normalize the direction
	if (bForce2D)
	{
		return PushTargetData.Direction.GetSafeNormal2D();
	}
	return PushTargetData.Direction.GetSafeNormal();
}

bool UPushStatics::IsPawnMovingOnGround(const APawn* Pawn)
{
	return Pawn->GetMovementComponent() ? Pawn->GetMovementComponent()->IsMovingOnGround() : false;
}

FVector UPushStatics::GetPawnGroundVelocity(const APawn* Pawn)
{
	// Factor incline into the velocity when on the ground
	const FVector& Velocity = Pawn->GetVelocity();
	return IsPawnMovingOnGround(Pawn) ? Velocity : FVector(Velocity.X, Velocity.Y, 0.f);
}

float UPushStatics::GetPawnGroundSpeed(const APawn* Pawn)
{
	// Factor incline into the velocity when on the ground
	const FVector& Velocity = GetPawnGroundVelocity(Pawn);
	return IsPawnMovingOnGround(Pawn) ? Velocity.Size() : Velocity.Size2D();
}

float UPushStatics::GetPushStrength(const APawn* Pushee, const FPushPawnActionParams& Params)
{
	// If no curve is supplied, return the scalar
	if (!Params.VelocityToStrengthCurve)
	{
		return Params.StrengthScalar;
	}

	// Get the speed of the pushee
	const float PusheeSpeed = GetPawnGroundSpeed(Pushee);

	// Get the strength from the curve and apply the scalar
	return Params.VelocityToStrengthCurve->GetFloatValue(PusheeSpeed) * Params.StrengthScalar;
}

float UPushStatics::GetPushStrengthSimple(const APawn* Pushee, const UCurveFloat* VelocityToStrengthCurve,
	float StrengthScalar)
{
	// If no curve is supplied, return the scalar
	if (!VelocityToStrengthCurve)
	{
		return StrengthScalar;
	}

	// Get the speed of the pushee
	const float PusheeSpeed = GetPawnGroundSpeed(Pushee);

	// Get the strength from the curve and apply the scalar
	return VelocityToStrengthCurve->GetFloatValue(PusheeSpeed) * StrengthScalar;
}

float UPushStatics::CalculatePushDirection(const FVector& Direction, const FRotator& BaseRotation)
{
	if (!Direction.IsNearlyZero())
	{
		const FMatrix RotMatrix = FRotationMatrix(BaseRotation);
		const FVector ForwardVector = RotMatrix.GetScaledAxis(EAxis::X);
		const FVector RightVector = RotMatrix.GetScaledAxis(EAxis::Y);
		const FVector NormalizedVel = Direction.GetSafeNormal2D();

		// get a cos(alpha) of forward vector vs velocity
		const float ForwardCosAngle = FVector::DotProduct(ForwardVector, NormalizedVel);
		// now get the alpha and convert to degree
		float ForwardDeltaDegree = FMath::RadiansToDegrees(FMath::Acos(ForwardCosAngle));

		// depending on where right vector is, flip it
		const float RightCosAngle = FVector::DotProduct(RightVector, NormalizedVel);
		if (RightCosAngle < 0)
		{
			ForwardDeltaDegree *= -1;
		}

		return ForwardDeltaDegree;
	}

	return 0.f;
}

EPushCardinal UPushStatics::GetPushDirection(const AActor* FromActor, const AActor* ToActor,
	EValidPushDirection& ValidPushDirection)
{
	ValidPushDirection = EValidPushDirection::InvalidDirection;

	// Get the direction from the pushee to the pusher
	const FVector Direction = (FromActor->GetActorLocation() - ToActor->GetActorLocation()).GetSafeNormal2D();

	// If the direction is nearly zero, default to forward
	if (Direction.IsNearlyZero())
	{
		return EPushCardinal::Forward;
	}

	ValidPushDirection = EValidPushDirection::ValidDirection;

	const float Rotation = CalculatePushDirection(Direction, ToActor->GetActorRotation());
	const float RotationAbs = FMath::Abs(Rotation);

	// Left or Right
	if (RotationAbs >= 67.5 && RotationAbs <= 112.5)
	{
		return Rotation > 0.f ? EPushCardinal::Right : EPushCardinal::Left;
	}

	// Forward
	if (RotationAbs <= 22.5f)
	{
		return EPushCardinal::Forward;
	}
	
	// Backward
	if (RotationAbs >= 157.5f)
	{
		return EPushCardinal::Backward;
	}

	// ForwardLeft or ForwardRight
	if (RotationAbs <= 67.5f)
	{
		return Rotation > 0.f ? EPushCardinal::ForwardRight : EPushCardinal::ForwardLeft;
	}

	// BackwardLeft or BackwardRight
	if (RotationAbs >= 112.5f)
	{
		return Rotation > 0.f ? EPushCardinal::BackwardRight : EPushCardinal::BackwardLeft;
	}

	// Default to forward
	return EPushCardinal::Forward;
}

FVector UPushStatics::GetPushPawnAcceleration(APawn* Pushee)
{
	const IPusheeInstigator* PusheeInstigator = Pushee ? Cast<IPusheeInstigator>(Pushee) : nullptr;
	return PusheeInstigator ? PusheeInstigator->GetPusheeAcceleration() : FVector::ZeroVector;
}

bool UPushStatics::IsPusheeAccelerating(APawn* Pushee)
{
	return IsPusheeAccelerating(GetPushPawnAcceleration(Pushee));
}

bool UPushStatics::IsPusheeAccelerating(const FVector& Acceleration)
{
	static constexpr float AccelThresholdSq = 100.f;  // Note: Acceleration is not normalized
	return Acceleration.SizeSquared() > AccelThresholdSq;
}

const float& UPushStatics::GetPushPawnScanRate(APawn* Pushee, const FPushPawnScanParams& ScanParams)
{
	return GetPushPawnScanRate(GetPushPawnAcceleration(Pushee), ScanParams);
}

const float& UPushStatics::GetPushPawnScanRate(const FVector& Acceleration, const FPushPawnScanParams& ScanParams)
{
	return IsPusheeAccelerating(Acceleration) ? ScanParams.ScanRateAccel : ScanParams.ScanRate;
}

float UPushStatics::GetPushPawnScanRange(APawn* Pushee, float BaseScanRange,
	const FPushPawnScanParams& ScanParams)
{
	return GetPushPawnScanRange(GetPushPawnAcceleration(Pushee), BaseScanRange, ScanParams);
}

float UPushStatics::GetPushPawnScanRange(const FVector& Acceleration, float BaseScanRange,
	const FPushPawnScanParams& ScanParams)
{
	// If the pushee is accelerating, return the scalar, otherwise return the base rate
	return BaseScanRange * IsPusheeAccelerating(Acceleration) ? ScanParams.ScanRangeAccelScalar : ScanParams.ScanRangeScalar;
}

float UPushStatics::GetMaxCapsuleSize(const AActor* Actor)
{
	if (Actor)
	{
		// Default capsule properties to ignore crouching or anything changing capsule height/radius
		const UCapsuleComponent* CapsuleComponent = Actor->GetRootComponent() ?
			Cast<UCapsuleComponent>(Actor->GetClass()->GetDefaultObject<AActor>()->GetRootComponent()) : nullptr;
		
		if (CapsuleComponent)
		{
			return FMath::Max<float>(CapsuleComponent->GetScaledCapsuleHalfHeight(), CapsuleComponent->GetScaledCapsuleRadius());
		}
	}
	return 0.f;
}

AActor* UPushStatics::GetActorFromPushTarget(TScriptInterface<IPusherTarget> PushTarget)
{
	if (UObject* Object = PushTarget.GetObject())
	{
		// If the object is an actor, return it
		if (AActor* Actor = Cast<AActor>(Object))
		{
			return Actor;
		}
		// If the object is a component, return the owner
		else if (const UActorComponent* ActorComponent = Cast<UActorComponent>(Object))
		{
			return ActorComponent->GetOwner();
		}
		// Otherwise, unimplemented
		else
		{
			unimplemented();
		}
	}

	return nullptr;
}

void UPushStatics::GetPushTargetsFromActor(AActor* Actor, TArray<TScriptInterface<IPusherTarget>>& OutPushTargets)
{
	// If the actor is directly Pusher, return that.
	const TScriptInterface<IPusherTarget> PushActor(Actor);
	if (PushActor)
	{
		OutPushTargets.Add(PushActor);
	}

	// If the actor isn't Pusher, it might have a component that has a Push interface.
	TArray<UActorComponent*> PushComponents = Actor ? Actor->GetComponentsByInterface(UPusheeInstigator::StaticClass()) : TArray<UActorComponent*>();
	for (UActorComponent* PushComponent : PushComponents)
	{
		OutPushTargets.Add(TScriptInterface<IPusherTarget>(PushComponent));
	}
}

void UPushStatics::AppendPushTargetsFromOverlapResults(const TArray<FOverlapResult>& OverlapResults, TArray<TScriptInterface<IPusherTarget>>& OutPushTargets)
{
	// Iterate over all the overlap results and gather their push targets
	for (const FOverlapResult& Overlap : OverlapResults)
	{
		// If the actor is a Pusher, return that.
		TScriptInterface<IPusherTarget> PushActor(Overlap.GetActor());
		if (PushActor)
		{
			OutPushTargets.AddUnique(PushActor);
		}

		// If the actor isn't Pusher, it might have a component that has a Push interface.
		TScriptInterface<IPusherTarget> PushComponent(Overlap.GetComponent());
		if (PushComponent)
		{
			OutPushTargets.AddUnique(PushComponent);
		}
	}
}

void UPushStatics::AppendPushTargetsFromHitResult(const FHitResult& HitResult, TArray<TScriptInterface<IPusherTarget>>& OutPushTargets)
{
	// If the actor is a Pusher, return that.
	const TScriptInterface<IPusherTarget> PushActor(HitResult.GetActor());
	if (PushActor)
	{
		OutPushTargets.AddUnique(PushActor);
	}

	// If the actor isn't Pusher, it might have a component that has a Push interface.
	const TScriptInterface<IPusherTarget> PushComponent(HitResult.GetComponent());
	if (PushComponent)
	{
		OutPushTargets.AddUnique(PushComponent);
	}
}
