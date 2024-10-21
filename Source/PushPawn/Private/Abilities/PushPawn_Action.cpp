// Copyright (c) Jared Taylor. All Rights Reserved


#include "Abilities/PushPawn_Action.h"

#include "AbilitySystemLog.h"
#include "PushStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/RootMotionSource.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PushPawn_Action)

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

	UCharacterMovementComponent* MovementComponent = Pushee->GetCharacterMovement();
	if (!MovementComponent || MovementComponent->MovementMode == MOVE_None)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);
		return false;
	}

	// Gather Push Strength
	const float Strength = UPushStatics::GetPushStrength(Pushee, PushParams);

	// Gather Push Direction
	static constexpr bool bForce2D = true;
	const FVector PushDirection = UPushStatics::GetPushDirectionFromEventData(EventData, bForce2D);

	// Apply the push Force - we bypass the ability system for this because it replicates 6 parameters we don't need!
	TSharedPtr<FRootMotionSource_ConstantForce> ConstantForce = MakeShared<FRootMotionSource_ConstantForce>();
	ConstantForce->InstanceName = TEXT("PushPawnForce");
	ConstantForce->AccumulateMode = ERootMotionAccumulateMode::Additive;
	ConstantForce->Priority = 5;
	ConstantForce->Force = PushDirection * Strength;
	ConstantForce->Duration = PushParams.Duration;
	ConstantForce->Settings.SetFlag(ERootMotionSourceSettingsFlags::IgnoreZAccumulate);
	uint16 RootMotionSourceID = MovementComponent->ApplyRootMotionSource(ConstantForce);

	// If we failed to apply the root motion source, cancel the ability
	if (RootMotionSourceID == (uint16)ERootMotionSourceID::Invalid)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);
		return false;
	}
	return true;
}
