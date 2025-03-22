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

bool UPushPawn_Action::ActivatePushPawnAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UPushPawn_Action::ActivatePushPawnAbility);

	const FGameplayEventData& EventData = *TriggerEventData;

	// Gather Pusher and Pushee
	UPushStatics::GetPushPawnsFromEventDataChecked<ACharacter, ACharacter>(EventData, Pushee, Pusher);

	// Check for null -- we can be destroyed during ability activation and crash!
	if (!IsValid(Pushee) || !IsValid(Pusher))
	{
		ABILITY_LOG(Error, TEXT("PushPawn_Action: Pushee or Pusher is null!"));
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);
		return false;
	}

	// Check for valid movement component and mode
	const UCharacterMovementComponent* MovementComponent = Pushee->GetCharacterMovement();
	if (!MovementComponent || MovementComponent->MovementMode == MOVE_None)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);
		return false;
	}
	
	// Gather Push Data
	UPushStatics::GetPushDataFromEventData(EventData, PushParams.bDistanceCheck2D, PushDirection,
		DistanceBetween, StrengthScalar, bOverrideStrength);
	
	// Push Strength
	NormalizedDistance = UPushStatics::GetNormalizedPushDistance(Pushee, Pusher, DistanceBetween);
	Strength = UPushStatics::CalculatePushStrength(Pushee, bOverrideStrength, StrengthScalar, NormalizedDistance, PushParams);

#if UE_ENABLE_DEBUG_DRAWING
	if (FPushPawnCVars::PushPawnActionDebugDraw > 0)  // Use WantsPushPawnActionDebugDraw() in derived classes
	{
		const bool bIsLocalPlayer = ActorInfo->IsLocallyControlled();
		if (FPushPawnCVars::PushPawnActionDebugDraw == 1 || bIsLocalPlayer)
		{
			DrawDebugDirectionalArrow(Pushee->GetWorld(), Pushee->GetActorLocation(),
				Pushee->GetActorLocation() + PushDirection * 100.f, 40.f, FColor::Magenta,
				false, 1.0f);
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
