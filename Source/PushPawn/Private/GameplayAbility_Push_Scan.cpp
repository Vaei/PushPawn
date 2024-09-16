// Copyright (c) Jared Taylor. All Rights Reserved


#include "GameplayAbility_Push_Scan.h"

#include "AbilitySystemComponent.h"
#include "IPush.h"
#include "PushStatics.h"
#include "Components/CapsuleComponent.h"
#include "NativeGameplayTags.h"
#include "Tasks/AbilityTask_GrantNearbyPush.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayAbility_Push_Scan)

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Ability_Push_Activate, "Ability.Push.Activate");

UGameplayAbility_Push_Scan::UGameplayAbility_Push_Scan(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	bServerRespectsRemoteAbilityCancellation = false;

	bDirectionIs2D = false;

	TraceChannel = ECC_Visibility;
}

void UGameplayAbility_Push_Scan::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	const AActor* AvatarActor = ActorInfo->AvatarActor.IsValid() ? ActorInfo->AvatarActor.Get() : nullptr;
	ScanRange = GetScanRange(AvatarActor);
	ScanRate = GetScanRate(AvatarActor);

	const UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponentFromActorInfo();
	if (AbilitySystem && AbilitySystem->GetOwnerRole() == ROLE_Authority)
	{
		UAbilityTask_GrantNearbyPush* Task = UAbilityTask_GrantNearbyPush::GrantAbilitiesForNearbyPushers(this, TraceChannel, ScanRange, ScanRate);
		Task->ReadyForActivation();
	}
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGameplayAbility_Push_Scan::UpdatePushs(const TArray<FPushOption>& PushOptions)
{
	CurrentOptions = PushOptions;
}

void UGameplayAbility_Push_Scan::TriggerPush()
{
	if (CurrentOptions.Num() == 0)
	{
		return;
	}

	const UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponentFromActorInfo();
	if (AbilitySystem)
	{
		const FPushOption& PushOption = CurrentOptions[0];

		AActor* PusheeInstigatorActor = GetAvatarActorFromActorInfo();
		AActor* PusherTargetActor = UPushStatics::GetActorFromPushTarget(PushOption.PusherTarget);

		// Usually the player character
		const IPusheeInstigator* PusheeInstigator = PusheeInstigatorActor ? Cast<IPusheeInstigator>(PusheeInstigatorActor) : nullptr;
		
		// Usually the AI character
		const IPusherTarget* PusherTarget = PusherTargetActor ? Cast<IPusherTarget>(PusherTargetActor) : nullptr;

		const bool bCanBePushed = PusheeInstigator && PusheeInstigator->CanBePushed(PusherTargetActor);
		const bool bCanPush = PusherTarget && PusherTarget->CanPush(PusheeInstigatorActor);

		if (!bCanBePushed || !bCanPush)
		{
			return;
		}

		// Use this to pass a Push direction, if we compute this later from the Payload Instigator or Target, it will
		// result in desync
		FHitResult DirectionHit;
		const FVector Direction = PushOption.PusheeActorLocation - PushOption.PusherActorLocation;
		DirectionHit.Normal = bDirectionIs2D ? Direction.GetSafeNormal2D() : Direction.GetSafeNormal();

		// Way too close to get a valid difference in direction
		if (Direction.IsNearlyZero(2.5f))
		{
			// This typically occurs on spawning, when their location is the exact same, if we move them back along their
			// forward vectors - they may just push forever in the same direction
			float RandomDegrees = FMath::RandRange(0.f, 360.f);
			const float RandAngle = FMath::DegreesToRadians(RandomDegrees);
			FVector NewDirection { FMath::Cos(RandAngle), FMath::Sin(RandAngle), 0.f };
			DirectionHit.Normal = NewDirection.GetSafeNormal();
		}

		// If we still don't have a valid direction, just push backwards
		if (!DirectionHit.Normal.IsNormalized() || !DirectionHit.Normal.IsUnit())
		{
			// Push the actor backwards if no valid direction was obtained, we don't use GetSafeNormal() because
			// we want to dictate the direction to something that makes sense for our use-case
			DirectionHit.Normal = -PushOption.PusheeForwardVector;
		}
		
		// Allow the target to customize the event data we're about to pass in, in case the ability needs custom data
		// that only the actor knows.
		FGameplayEventData Payload;
		Payload.EventTag = TAG_Ability_Push_Activate;
		Payload.Instigator = PusheeInstigatorActor;
		Payload.Target = PusherTargetActor;
		Payload.ContextHandle = GetContextFromOwner(FGameplayAbilityTargetDataHandle());
		Payload.ContextHandle.AddHitResult(DirectionHit);

		// If needed we allow the Push target to manipulate the event data
		PushOption.PusherTarget->CustomizePushEventData(TAG_Ability_Push_Activate, Payload);

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
			TAG_Ability_Push_Activate,
			&Payload,
			*PushOption.TargetAbilitySystem
		);
	}
}

float UGameplayAbility_Push_Scan::GetScanRange_Implementation(const AActor* AvatarActor) const
{
	return GetMaxCapsuleSize(AvatarActor);
}

float UGameplayAbility_Push_Scan::GetScanRate_Implementation(const AActor* AvatarActor) const
{
	return ScanRate;
}

float UGameplayAbility_Push_Scan::GetMaxCapsuleSize(const AActor* AvatarActor) const
{
	if (AvatarActor)
	{
		// Default capsule properties to ignore crouching or anything changing capsule height/radius
		const UCapsuleComponent* CapsuleComponent = AvatarActor->GetRootComponent() ?
			Cast<UCapsuleComponent>(AvatarActor->GetClass()->GetDefaultObject<AActor>()->GetRootComponent()) : nullptr;
		
		if (CapsuleComponent)
		{
			return FMath::Max<float>(CapsuleComponent->GetScaledCapsuleHalfHeight(), CapsuleComponent->GetScaledCapsuleRadius());
		}
	}
	return 0.f;
}
