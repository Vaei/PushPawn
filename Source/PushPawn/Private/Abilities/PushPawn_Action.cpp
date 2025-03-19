// Copyright (c) Jared Taylor. All Rights Reserved


#include "Abilities/PushPawn_Action.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Tasks/AbilityTask_PushPawnForce.h"
#include "PushStatics.h"

#include "AbilitySystemLog.h"
#include "DrawDebugHelpers.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PushPawn_Action)

namespace FPushPawnCVars
{
#if UE_ENABLE_DEBUG_DRAWING
	static int32 PushPawnActionDebugDraw = 0;
	FAutoConsoleVariableRef CVarPushPawnActionDebugDraw(
		TEXT("p.PushPawn.Action.Debug.Draw"),
		PushPawnActionDebugDraw,
		TEXT("Optionally draw debug for PushPawn Action (Magenta).\n")
		TEXT("0: Disable, 1: Enable for all, 2: Enable for local player only"),
		ECVF_Default);
#endif
}

float SafeDivide(float A, float B)
{
	// Prevent potential divide by zero issues
	return B != 0.f ? (A / B) : 0.f;
}

bool UPushPawn_Action::ActivatePushPawnAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UPushPawn_Action::ActivatePushPawnAbility);

	const FGameplayEventData& EventData = *TriggerEventData;

	// Gather Pusher and Pushee
	const ACharacter* Pushee = nullptr;
	const AActor* Pusher = nullptr;
	UPushStatics::GetPushPawnsFromEventDataChecked<ACharacter, AActor>(EventData, Pushee, Pusher);

	if (!Pushee || !Pusher)
	{
		ABILITY_LOG(Error, TEXT("PushPawn_Action: Pushee or Pusher is null!"));
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);
		return false;
	}

	const UCharacterMovementComponent* MovementComponent = Pushee->GetCharacterMovement();
	if (!MovementComponent || MovementComponent->MovementMode == MOVE_None)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);
		return false;
	}
	
	// Gather Push Direction
	static constexpr bool bForce2D = true;
	FVector PushDirection = FVector::ZeroVector;
	float DistanceBetween = 0.f;
	UPushStatics::GetPushDirectionAndDistanceBetweenFromEventData(EventData, bForce2D, PushDirection, DistanceBetween);

	// Normalize DistanceBetween (might be able to move concept this into the scan and simplify it as CapsuleOverlapPct or something).
	float CombinedRadius = Pushee->GetSimpleCollisionRadius() +  Pusher->GetSimpleCollisionRadius();
	float NormalizedDistance = SafeDivide(DistanceBetween, CombinedRadius);
	
	// Gather Push Strength
	const float Strength = UPushStatics::GetPushStrength(Pushee, NormalizedDistance, PushParams);

#if UE_ENABLE_DEBUG_DRAWING
	if (FPushPawnCVars::PushPawnActionDebugDraw > 0)  // Use WantsPushPawnActionDebugDraw() in derived classes
	{
		const bool bIsLocalPlayer = ActorInfo->IsLocallyControlled();
		if (FPushPawnCVars::PushPawnActionDebugDraw == 1 || bIsLocalPlayer)
		{
			DrawDebugDirectionalArrow(Pushee->GetWorld(), Pushee->GetActorLocation(), Pushee->GetActorLocation() + PushDirection * 100.f, 40.f, FColor::Magenta, false, 1.0f);
		}
	}
#endif

	// Apply Push Force Task
	static constexpr bool bIsAdditive = true;
	static constexpr bool bEnableGravity = true;
	UAbilityTask_PushPawnForce* PushTask = UAbilityTask_PushPawnForce::ApplyPushPawnForce(
		this, PushDirection, Strength, PushParams.Duration, bIsAdditive, bEnableGravity);

	// Bind EndAbility to OnFinish
	PushTask->OnFinish.AddDynamic(this, &UPushPawn_Action::OnPushTaskFinished);

	// Activate the Push Task
	PushTask->SharedInitAndApply();

	return true;
}

void UPushPawn_Action::OnPushTaskFinished()
{
	K2_EndAbility();
}
