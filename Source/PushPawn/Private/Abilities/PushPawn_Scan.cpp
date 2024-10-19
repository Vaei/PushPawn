// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/PushPawn_Scan.h"

#include "PushQuery.h"
#include "Tasks/AbilityTask_WaitForPushTargets_CapsuleTrace.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PushPawn_Scan)

void UPushPawn_Scan::ActivatePushPawnAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UPushPawn_Scan::ActivatePushPawnAbility);

	Super::ActivatePushPawnAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Gather parameters
	FPushQuery Query { GetAvatarActorFromActorInfo() };
	FGameplayAbilityTargetingLocationInfo StartLocation = MakeTargetLocationInfoFromOwnerActor();

	// Create the task
	UAbilityTask_WaitForPushTargets_CapsuleTrace* Task = UAbilityTask_WaitForPushTargets_CapsuleTrace::WaitForPushTargets_CapsuleTrace(
		this, Query, StartLocation, ScanParams);

	// Bind the event
	Task->PushObjectsChanged.AddDynamic(this, &UPushPawn_Scan::OnPushObjectsChanged);

	// Activate the task
	Task->ReadyForActivation();
}

void UPushPawn_Scan::OnPushObjectsChanged(const TArray<FPushOption>& PushOptions)
{
	UpdatePushes(PushOptions);
	TriggerPush();
}
