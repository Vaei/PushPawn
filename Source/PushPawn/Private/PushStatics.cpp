// Copyright Epic Games, Inc. All Rights Reserved.

#include "PushStatics.h"
#include "IPush.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PushStatics)

UPushStatics::UPushStatics()
	: Super(FObjectInitializer::Get())
{
}

AActor* UPushStatics::GetActorFromPushTarget(TScriptInterface<IPusherTarget> PushTarget)
{
	if (UObject* Object = PushTarget.GetObject())
	{
		if (AActor* Actor = Cast<AActor>(Object))
		{
			return Actor;
		}
		else if (const UActorComponent* ActorComponent = Cast<UActorComponent>(Object))
		{
			return ActorComponent->GetOwner();
		}
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

	// If the actor isn't Push, it might have a component that has a Push interface.
	TArray<UActorComponent*> PushComponents = Actor ? Actor->GetComponentsByInterface(UPusheeInstigator::StaticClass()) : TArray<UActorComponent*>();
	for (UActorComponent* PushComponent : PushComponents)
	{
		OutPushTargets.Add(TScriptInterface<IPusherTarget>(PushComponent));
	}
}

void UPushStatics::AppendPushTargetsFromOverlapResults(const TArray<FOverlapResult>& OverlapResults, TArray<TScriptInterface<IPusherTarget>>& OutPushTargets)
{
	for (const FOverlapResult& Overlap : OverlapResults)
	{
		TScriptInterface<IPusherTarget> PushActor(Overlap.GetActor());
		if (PushActor)
		{
			OutPushTargets.AddUnique(PushActor);
		}

		TScriptInterface<IPusherTarget> PushComponent(Overlap.GetComponent());
		if (PushComponent)
		{
			OutPushTargets.AddUnique(PushComponent);
		}
	}
}

void UPushStatics::AppendPushTargetsFromHitResult(const FHitResult& HitResult, TArray<TScriptInterface<IPusherTarget>>& OutPushTargets)
{
	const TScriptInterface<IPusherTarget> PushActor(HitResult.GetActor());
	if (PushActor)
	{
		OutPushTargets.AddUnique(PushActor);
	}

	const TScriptInterface<IPusherTarget> PushComponent(HitResult.GetComponent());
	if (PushComponent)
	{
		OutPushTargets.AddUnique(PushComponent);
	}
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
	const FVector Direction = (FromActor->GetActorLocation() - ToActor->GetActorLocation()).GetSafeNormal2D();
	
	if (Direction.IsNearlyZero())
	{
		return EPushCardinal::Forward;
	}

	ValidPushDirection = EValidPushDirection::ValidDirection;

	const float Rotation = CalculatePushDirection(Direction, ToActor->GetActorRotation());
	const float RotationAbs = FMath::Abs(Rotation);

	if (RotationAbs >= 67.5 && RotationAbs <= 112.5)
	{
		return Rotation > 0.f ? EPushCardinal::Right : EPushCardinal::Left;
	}

	if (RotationAbs <= 22.5f)
	{
		return EPushCardinal::Forward;
	}
	
	// Backward
	if (RotationAbs >= 157.5f)
	{
		return EPushCardinal::Backward;
	}

	if (RotationAbs <= 67.5f)
	{
		return Rotation > 0.f ? EPushCardinal::ForwardRight : EPushCardinal::ForwardLeft;
	}

	if (RotationAbs >= 112.5f)
	{
		return Rotation > 0.f ? EPushCardinal::BackwardRight : EPushCardinal::BackwardLeft;
	}

	return EPushCardinal::Forward;
}
