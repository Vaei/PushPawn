// Copyright (c) Jared Taylor. All Rights Reserved


#include "Abilities/PushPawn_Scan.h"

#include "Tasks/AbilityTask_PushPawnScan.h"
#include "PushQuery.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PushPawn_Scan)

bool UPushPawn_Scan::ActivatePushPawnAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UPushPawn_Scan::ActivatePushPawnAbility);

	if (Super::ActivatePushPawnAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData))
	{
		// Gather parameters
		FPushQuery Query { GetAvatarActorFromActorInfo() };
		FGameplayAbilityTargetingLocationInfo StartLocation = MakeTargetLocationInfoFromOwnerActor();

		// Create the task
		UAbilityTask_PushPawnScan* Task = UAbilityTask_PushPawnScan::PushPawnScan(
			this, Query, StartLocation, ScanParams);

		// Bind the event
		Task->PushObjectsChanged.AddDynamic(this, &ThisClass::OnPushObjectsChanged);

		// Activate the task
		Task->ReadyForActivation();

		return true;
	}
	return false;
}

void UPushPawn_Scan::OnPushObjectsChanged(const TArray<FPushOption>& PushOptions)
{
	UpdatePushes(PushOptions);
	TriggerPush();
}
