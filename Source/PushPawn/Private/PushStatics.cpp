// Copyright (c) Jared Taylor. All Rights Reserved

#include "PushStatics.h"

#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Abilities/PushPawnAbilityTargetData.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PusheeComponent.h"
#include "Components/SphereComponent.h"
#include "IPush.h"
#include "PushQuery.h"

#include "Engine/OverlapResult.h"
#include "Curves/CurveFloat.h"

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

bool UPushStatics::GetDefaultCapsuleRootComponent(const AActor* Actor, float& CapsuleRadius, float& CapsuleHalfHeight)
{
	CapsuleRadius = 0.f;
	CapsuleHalfHeight = 0.f;
	
	const AActor* DefaultActor = Actor ? Actor->GetClass()->GetDefaultObject<AActor>() : nullptr;
	if (DefaultActor && DefaultActor->GetRootComponent())
	{
		if (const UCapsuleComponent* CapsuleComponent = Cast<UCapsuleComponent>(DefaultActor->GetRootComponent()))
		{
			CapsuleRadius = CapsuleComponent->GetUnscaledCapsuleRadius();
			CapsuleHalfHeight = CapsuleComponent->GetUnscaledCapsuleHalfHeight();
			return true;
		}
	}
	return false;
}

bool UPushStatics::IsPawnMovingOnGround(const APawn* Pawn)
{
	return Pawn->GetMovementComponent() ? Pawn->GetMovementComponent()->IsMovingOnGround() : false;
}

bool UPushStatics::IsPusheeMovingOnGround(const IPusheeInstigator* Pushee)
{
	return Pushee->IsPusheeMovingOnGround();
}

FVector UPushStatics::GetPawnGroundVelocity(const APawn* Pawn)
{
	// Factor incline into the velocity when on the ground
	const FVector& Velocity = Pawn->GetVelocity();
	return IsPawnMovingOnGround(Pawn) ? Velocity : FVector(Velocity.X, Velocity.Y, 0.f);
}

FVector UPushStatics::GetPusheeGroundVelocity(const IPusheeInstigator* Pushee)
{
	const FVector& Velocity = Pushee->GetPusheeVelocity();
	return Pushee->IsPusheeMovingOnGround() ? Velocity : FVector(Velocity.X, Velocity.Y, 0.f);
}

float UPushStatics::GetPawnGroundSpeed(const APawn* Pawn)
{
	// Factor incline into the velocity when on the ground
	return GetPawnGroundVelocity(Pawn).Size();
}

float UPushStatics::GetPusheeGroundSpeed(const IPusheeInstigator* Pushee)
{
	// Factor incline into the velocity when on the ground
	return GetPusheeGroundVelocity(Pushee).Size();
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

IPusheeInstigator* UPushStatics::GetPusheeInstigator(AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}
	
	if (IPusheeInstigator* Interface = Cast<IPusheeInstigator>(Actor))
	{
		return Interface;
	}

	if (auto* Component = Actor->GetComponentByClass<UPusheeComponent>())
	{
		return Cast<IPusheeInstigator>(Component);
	}

	return nullptr;
}

const IPusheeInstigator* UPushStatics::GetPusheeInstigator(const AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}
	
	if (const IPusheeInstigator* Interface = Cast<IPusheeInstigator>(Actor))
	{
		return Interface;
	}

	if (auto* Component = Actor->GetComponentByClass<UPusheeComponent>())
	{
		return Cast<IPusheeInstigator>(Component);
	}

	return nullptr;
}

FVector UPushStatics::GetPushPawnAcceleration(const IPusheeInstigator* Pushee)
{
	return Pushee ? Pushee->GetPusheeAcceleration().GetSafeNormal() : FVector::ZeroVector;
}

FVector UPushStatics::GetPushPawnAcceleration(APawn* Pushee)
{
	const IPusheeInstigator* PusheeInstigator = GetPusheeInstigator(Pushee);
	return PusheeInstigator ? PusheeInstigator->GetPusheeAcceleration().GetSafeNormal() : FVector::ZeroVector;
}

bool UPushStatics::IsPusheeAccelerating(const IPusheeInstigator* Pushee)
{
	return IsPusheeAccelerating(GetPushPawnAcceleration(Pushee));
}

bool UPushStatics::IsPusheeAccelerating(APawn* Pushee)
{
	return IsPusheeAccelerating(GetPushPawnAcceleration(Pushee));
}

bool UPushStatics::IsPusheeAccelerating(const FVector& Acceleration)
{
	return !Acceleration.GetSafeNormal().IsNearlyZero(0.1f);
}

const float& UPushStatics::GetPushPawnScanRate(const IPusheeInstigator* Pushee, const FPushPawnScanParams& ScanParams)
{
	return GetPushPawnScanRate(GetPushPawnAcceleration(Pushee), ScanParams);
}

const float& UPushStatics::GetPushPawnScanRate(APawn* Pushee, const FPushPawnScanParams& ScanParams)
{
	return GetPushPawnScanRate(GetPushPawnAcceleration(Pushee), ScanParams);
}

const float& UPushStatics::GetPushPawnScanRate(const FVector& Acceleration, const FPushPawnScanParams& ScanParams)
{
	return IsPusheeAccelerating(Acceleration) ? ScanParams.ScanRateAccel : ScanParams.ScanRate;
}

float UPushStatics::GetPushPawnScanRange(const IPusheeInstigator* Pushee, float BaseScanRange,
	const FPushPawnScanParams& ScanParams)
{
	return GetPushPawnScanRange(GetPushPawnAcceleration(Pushee), BaseScanRange, ScanParams);
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
	return BaseScanRange * (IsPusheeAccelerating(Acceleration) ? ScanParams.ScanRangeAccelScalar : ScanParams.ScanRangeScalar);
}

bool UPushStatics::GatherPushOptions(const TSubclassOf<UGameplayAbility>& PushAbilityToGrant,
	const APawn* PusherPawn, const FPushQuery& PushQuery, const FPushOptionBuilder& OptionBuilder)
{
	if (PushAbilityToGrant && PusherPawn && PushQuery.RequestingAvatar.IsValid())
	{
		FPushOption Push;
		Push.PushAbilityToGrant = PushAbilityToGrant;
		Push.PusheeActorLocation = PushQuery.RequestingAvatar->GetActorLocation();
		Push.PusheeForwardVector = PusherPawn->GetActorForwardVector();
		Push.PusherActorLocation = PusherPawn->GetActorLocation();
		OptionBuilder.AddPushOption(Push);
		return true;
	}
	return false;
}

EPushCollisionType UPushStatics::GetPusheeCollisionShapeType(const AActor* Actor)
{
	const USceneComponent* RootComponent = Actor->GetRootComponent();
	if (RootComponent->IsA<UCapsuleComponent>())
	{
		return EPushCollisionType::Capsule;
	}
	else if (RootComponent->IsA<UBoxComponent>())
	{
		return EPushCollisionType::Box;
	}
	else if (RootComponent->IsA<USphereComponent>())
	{
		return EPushCollisionType::Sphere;
	}
	return EPushCollisionType::None;
}

FCollisionShape UPushStatics::GetDefaultPusheeCollisionShape(const AActor* Actor, FQuat& OutShapeRotation, EPushCollisionType OptionalShapeType, USceneComponent* OptionalComponent)
{
	if (OptionalShapeType == EPushCollisionType::None)
	{
		OptionalShapeType = GetPusheeCollisionShapeType(Actor);
	}

	// Use the default root component if no specific component is supplied
	USceneComponent* Component = OptionalComponent ? OptionalComponent : Actor->GetClass()->GetDefaultObject<AActor>()->GetRootComponent();
	OutShapeRotation = Component->GetComponentQuat();
	switch (OptionalShapeType)
	{
		case EPushCollisionType::Capsule:
		{
			const UCapsuleComponent* CapsuleComponent = CastChecked<UCapsuleComponent>(Component);
			return FCollisionShape::MakeCapsule(CapsuleComponent->GetScaledCapsuleRadius(), CapsuleComponent->GetScaledCapsuleHalfHeight());
		}
		case EPushCollisionType::Box:
		{
			const UBoxComponent* BoxComponent = CastChecked<UBoxComponent>(Component);
			return FCollisionShape::MakeBox(BoxComponent->GetScaledBoxExtent());
		}
		case EPushCollisionType::Sphere:
		{
			const USphereComponent* SphereComponent = CastChecked<USphereComponent>(Component);
			return FCollisionShape::MakeSphere(SphereComponent->GetScaledSphereRadius());
		}
		default: return {};
	}
}

float UPushStatics::GetMaxDefaultCollisionShapeSize(const AActor* Actor, EPushCollisionType SpecificShapeType)
{
	if (Actor)
	{
		FQuat ShapeRotation;
		FCollisionShape CollisionShape = GetDefaultPusheeCollisionShape(Actor, ShapeRotation, SpecificShapeType, nullptr);
		if (!CollisionShape.IsNearlyZero())
		{
			switch (CollisionShape.ShapeType)
			{
				case ECollisionShape::Box: return CollisionShape.GetExtent().GetAbsMax();
				case ECollisionShape::Sphere: return CollisionShape.GetSphereRadius();
				case ECollisionShape::Capsule: return FMath::Max<float>(CollisionShape.GetCapsuleRadius(), CollisionShape.GetCapsuleHalfHeight());
				default: return 0.f;
			}
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
