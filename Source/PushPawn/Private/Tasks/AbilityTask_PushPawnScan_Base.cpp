// Copyright (c) Jared Taylor. All Rights Reserved

#include "Tasks/AbilityTask_PushPawnScan_Base.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AbilityTask_PushPawnScan_Base)

UAbilityTask_PushPawnScan_Base::UAbilityTask_PushPawnScan_Base(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, TraceChannel(ECC_Visibility)
{
}

void UAbilityTask_PushPawnScan_Base::ShapeTrace(FHitResult& OutHitResult, const UWorld* World, const FVector& Center,
	const FQuat& Rotation, const ECollisionChannel ChannelName, const FCollisionQueryParams& Params, const FCollisionShape& Shape)
{
	check(World);

	// Make it move so the sweep registers
	const FVector End = Center + FVector::UpVector * -0.1f;

	// Perform the trace
	OutHitResult = FHitResult();
	TArray<FHitResult> HitResults;
	World->SweepMultiByChannel(HitResults, Center, End, Rotation, ChannelName, Shape, Params);

	// Set the trace start and end
	OutHitResult.TraceStart = Center;
	OutHitResult.TraceEnd = End;

	// If we hit something, set the first hit result
	if (HitResults.Num() > 0)
	{
		OutHitResult = HitResults[0];
	}
}

void UAbilityTask_PushPawnScan_Base::UpdatePushOptions(const FPushQuery& PushQuery, const TArray<TScriptInterface<IPusherTarget>>& PushTargets)
{
	// Iterate over all the push targets and gather their push options
	TArray<FPushOption> NewOptions;
	for (const TScriptInterface<IPusherTarget>& PushTarget : PushTargets)
	{
		// Gather the push options
		TArray<FPushOption> PushOptions;
		FPushOptionBuilder PushBuilder(PushTarget, PushOptions);
		PushTarget->GatherPushOptions(PushQuery, PushBuilder);

		// Iterate over the options and update their parameters and filter out any that can't be activated
		for (FPushOption& Option : PushOptions)
		{
			const FGameplayAbilitySpec* PushAbilitySpec = nullptr;

			// If there is a handle and a target ability system, we're triggering the ability on the target
			if (Option.TargetAbilitySystem && Option.TargetPushAbilityHandle.IsValid())
			{
				// Find the spec
				PushAbilitySpec = Option.TargetAbilitySystem->FindAbilitySpecFromHandle(Option.TargetPushAbilityHandle);
			}
			// If there's a Push ability then we're activating it on ourselves
			else if (Option.PushAbilityToGrant)
			{
				// Find the spec
				PushAbilitySpec = AbilitySystemComponent->FindAbilitySpecFromClass(Option.PushAbilityToGrant);

				if (PushAbilitySpec)
				{
					// Update the option
					Option.TargetAbilitySystem = AbilitySystemComponent.Get();
					Option.TargetPushAbilityHandle = PushAbilitySpec->Handle;
				}
			}

			// Filter any options that we can't activate right now for whatever reason
			if (PushAbilitySpec)
			{
				if (PushAbilitySpec->Ability->CanActivateAbility(PushAbilitySpec->Handle, AbilitySystemComponent->AbilityActorInfo.Get()))
				{
					NewOptions.Add(Option);
				}
			}
		}
	}

	// Sort the options
	bool bOptionsChanged = false;
	if (NewOptions.Num() == CurrentOptions.Num())
	{
		NewOptions.Sort();

		for (int32 OptionIndex = 0; OptionIndex < NewOptions.Num(); OptionIndex++)
		{
			const FPushOption& NewOption = NewOptions[OptionIndex];
			const FPushOption& CurrentOption = CurrentOptions[OptionIndex];

			if (NewOption != CurrentOption)
			{
				bOptionsChanged = true;
				break;
			}
		}
	}
	else
	{
		bOptionsChanged = true;
	}

	// If the options have changed, update the options and broadcast the change
	if (bOptionsChanged)
	{
		CurrentOptions = NewOptions;
		PushObjectsChanged.Broadcast(CurrentOptions);
	}
}

void UAbilityTask_PushPawnScan_Base::OnDestroy(bool bInOwnerFinished)
{
	// #KillPendingKill Clear ability reference so we don't hold onto it and GC can delete it.
	PushScanAbility = nullptr;

	Super::OnDestroy(bInOwnerFinished);
}
