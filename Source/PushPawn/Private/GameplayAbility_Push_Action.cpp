// Copyright (c) Jared Taylor. All Rights Reserved


#include "GameplayAbility_Push_Action.h"

UGameplayAbility_Push_Action::UGameplayAbility_Push_Action(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bServerRespectsRemoteAbilityCancellation = false;
	bRetriggerInstancedAbility = true;
}

float UGameplayAbility_Push_Action::CalculateDirection(const FVector& Direction, const FRotator& BaseRotation)
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


EPushCardinal UGameplayAbility_Push_Action::GetPushDirection(const AActor* FromActor, const AActor* ToActor,
	EValidPushDirection& ValidPushDirection)
{
	ValidPushDirection = EValidPushDirection::InvalidDirection;
	const FVector Direction = (FromActor->GetActorLocation() - ToActor->GetActorLocation()).GetSafeNormal2D();
	
	if (Direction.IsNearlyZero())
	{
		return EPushCardinal::Forward;
	}

	ValidPushDirection = EValidPushDirection::ValidDirection;

	const float Rotation = CalculateDirection(Direction, ToActor->GetActorRotation());
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
