// Copyright (c) Jared Taylor. All Rights Reserved


#include "Abilities/PushPawn_Scan_Base.h"
#include "IPush.h"
#include "PushStatics.h"
#include "NativeGameplayTags.h"
#include "PushPawnTags.h"
#include "Abilities/PushPawnAbilityTargetData.h"
#include "Tasks/AbilityTask_GrantPushAbility.h"
#include "AbilitySystemComponent.h"
#include "Tasks/AbilityTask_PushPawnScan.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PushPawn_Scan_Base)

UPushPawn_Scan_Base::UPushPawn_Scan_Base(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BaseScanRange = 0.f;
}

void UPushPawn_Scan_Base::OnGameplayTaskInitialized(UGameplayTask& Task)
{
	// If the task is a UPushPawn_Scan, set the PushScanAbility to this
	// Allows the task to call back to this ability, specifically ShouldWaitForNetSync()
	if (auto* AbilityTask = Cast<UAbilityTask_PushPawnScan>(&Task))
	{
		AbilityTask->PushScanAbility = this;
	}

	Super::OnGameplayTaskInitialized(Task);
}

bool UPushPawn_Scan_Base::ActivatePushPawnAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// Get the Avatar actor
	const AActor* AvatarActor = ActorInfo->AvatarActor.IsValid() ? ActorInfo->AvatarActor.Get() : nullptr;

	// Get the base scan range
	BaseScanRange = GetBaseScanRange(AvatarActor);

	// If we don't have any scan range, we can't do anything, we will _never_ succeed.
	if (!ensureMsgf(!FMath::IsNearlyZero(BaseScanRange), TEXT("BaseScanRange is zero, this will cause the scan to fail.")))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);
		return false;
	}

	// Get the ability system component
	const UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponentFromActorInfo();

	// If we have an ability system, and we're the authority, create the task to grant abilities for nearby pushers
	if (AbilitySystem && AbilitySystem->GetOwnerRole() == ROLE_Authority)
	{
		UAbilityTask_GrantPushAbility* Task = UAbilityTask_GrantPushAbility::GrantAbilitiesForNearbyPushers(this, ScanParams, BaseScanRange);
		Task->ReadyForActivation();
	}
	return true;
}

void UPushPawn_Scan_Base::UpdatePushes(const TArray<FPushOption>& PushOptions)
{
	CurrentOptions = PushOptions;
}

void UPushPawn_Scan_Base::TriggerPush()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UPushPawn_Scan_Base::TriggerPush);

	// If we have no options, we can't do anything
	if (CurrentOptions.Num() == 0)
	{
		return;
	}

	// Get the first push option
	const FPushOption& PushOption = CurrentOptions[0];

	// Get the pushee and pusher actors
	AActor* PusheeInstigatorActor = GetAvatarActorFromActorInfo();
	AActor* PusherTargetActor = UPushStatics::GetActorFromPushTarget(PushOption.PusherTarget);

	// The ability instigator that is being pushed
	const IPusheeInstigator* PusheeInstigator = UPushStatics::GetPusheeInstigator(PusheeInstigatorActor);
	
	// The ability target that does the pushing
	const IPusherTarget* PusherTarget = PusherTargetActor ? Cast<IPusherTarget>(PusherTargetActor) : nullptr;

	// Check if the pushee can be pushed by the pusher
	const bool bCanBePushed = PusheeInstigator && PusheeInstigator->CanBePushedBy(PusherTargetActor);

	// Check if the pusher can push the pushee
	const bool bCanPush = PusherTarget && PusherTarget->CanPushPawn(PusheeInstigatorActor);

	// If the pushee can't be pushed or the pusher can't push, we can't do anything
	if (!bCanBePushed || !bCanPush)
	{
		return;
	}

	// Use this to pass a Push direction, if we compute this later from the Payload Instigator or Target, it will
	// result in de-sync
	FVector Direction = PushOption.PusheeActorLocation - PushOption.PusherActorLocation;
	Direction = ScanParams.bDirectionIs2D ? Direction.GetSafeNormal2D() : Direction.GetSafeNormal();

	// Way too close to get a valid difference in direction
	if (Direction.IsNearlyZero(2.5f))
	{
		// This typically occurs on spawning, when their location is the exact same, if we move them back along their
		// forward vectors - they may just push forever in the same direction
		float RandomDegrees = FMath::RandRange(0.f, 360.f);
		const float RandAngle = FMath::DegreesToRadians(RandomDegrees);
		FVector NewDirection { FMath::Cos(RandAngle), FMath::Sin(RandAngle), 0.f };
		Direction = NewDirection.GetSafeNormal();
	}

	// If we still don't have a valid direction, just push backwards
	if (!Direction.IsNormalized() || !Direction.IsUnit())
	{
		// Push the actor backwards if no valid direction was obtained, we don't use GetSafeNormal() because
		// we want to dictate the direction to something that makes sense for our use-case
		Direction = -PushOption.PusheeForwardVector;
	}
	
	// Allow the target to customize the event data we're about to pass in, in case the ability needs custom data
	// that only the actor knows.
	FPushPawnAbilityTargetData* TargetData = new FPushPawnAbilityTargetData(Direction);

	// The payload data for the Push ability
	FGameplayEventData Payload;
	Payload.EventTag = FPushPawnTags::PushPawn_PushAbility_Activate;
	Payload.Instigator = PusheeInstigatorActor;
	Payload.Target = PusherTargetActor;
	Payload.TargetData.Add(TargetData);

	// If needed we allow the Push target to manipulate the event data
	PushOption.PusherTarget->CustomizePushEventData(FPushPawnTags::PushPawn_PushAbility_Activate, Payload);

	// Grab the target actor off the payload we're going to use it as the 'avatar' for the Push, and the
	// source PushTarget actor as the owner actor.
	AActor* TargetActor = const_cast<AActor*>(Payload.Target.Get());

	// The actor info needed for the Push.
	FGameplayAbilityActorInfo ActorInfo;
	ActorInfo.InitFromActor(PusherTargetActor, TargetActor, PushOption.TargetAbilitySystem);

	// Trigger the ability using event tag.
	PushOption.TargetAbilitySystem->TriggerAbilityFromGameplayEvent(
		PushOption.TargetPushAbilityHandle,
		&ActorInfo,
		FPushPawnTags::PushPawn_PushAbility_Activate,
		&Payload,
		*PushOption.TargetAbilitySystem
	);

	TriggeredPushesSinceLastNetSync++;
	LastPushTime = GetWorld()->GetTimeSeconds();
}

float UPushPawn_Scan_Base::GetBaseScanRange(const AActor* AvatarActor) const
{
	// If we allow blueprint implementation, call the blueprint method instead
	if (bAllowBlueprintImplementation)
	{
		return K2_GetBaseScanRange(AvatarActor);
	}

	// Default to max collision shape size
	return UPushStatics::GetMaxDefaultCollisionShapeSize(AvatarActor);
}

bool UPushPawn_Scan_Base::ShouldWaitForNetSync() const
{
	// If a push occurred recently, don't sync just yet
	if (MinNetSyncDelay > 0.f && LastPushTime >= 0.f &&
		GetWorld()->TimeSince(LastPushTime) < MinNetSyncDelay)
	{
		return false;
	}

	// If too much time has passed since the last push, sync
	if (NetSyncDelayAfterPush > 0.f && TriggeredPushesSinceLastNetSync > 0 &&
		LastPushTime >= 0.f && GetWorld()->TimeSince(LastPushTime) >= NetSyncDelayAfterPush)
	{
		return true;
	}

	// If we've exceeded the max pushes, sync
	if (MaxPushesUntilNetSync == 0)
	{
		return false;
	}
	return TriggeredPushesSinceLastNetSync > MaxPushesUntilNetSync;
}

void UPushPawn_Scan_Base::ConsumeWaitForNetSync()
{
	TriggeredPushesSinceLastNetSync = 0;
	LastPushTime = -1.f;
}

float UPushPawn_Scan_Base::K2_GetBaseScanRange_Implementation(const AActor* AvatarActor) const
{
	// Default to max collision shape size
	return UPushStatics::GetMaxDefaultCollisionShapeSize(AvatarActor);
}
