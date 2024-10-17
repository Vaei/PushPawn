// Copyright (c) Jared Taylor. All Rights Reserved

#include "Tasks/AbilityTask_WaitForPushTargets.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AbilityTask_WaitForPushTargets)

UAbilityTask_WaitForPushTargets::UAbilityTask_WaitForPushTargets(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, TraceChannel(ECC_Visibility)
{
}

void UAbilityTask_WaitForPushTargets::ShapeTrace(FHitResult& OutHitResult, const UWorld* World, const FVector& Center,
	ECollisionChannel ChannelName, const FCollisionQueryParams& Params, const FCollisionShape& Shape)
{
	check(World);

	// Make it move so the sweep registers
	const FVector End = Center + FVector::UpVector * -0.1f;

	OutHitResult = FHitResult();
	TArray<FHitResult> HitResults;
	World->SweepMultiByChannel(HitResults, Center, End, FQuat::Identity, ChannelName, Shape, Params);

	OutHitResult.TraceStart = Center;
	OutHitResult.TraceEnd = End;

	if (HitResults.Num() > 0)
	{
		OutHitResult = HitResults[0];
	}
}

void UAbilityTask_WaitForPushTargets::DetectNearbyTargets(const AActor* InSourceActor, FCollisionQueryParams Params,
	const FVector& TraceStart, float MaxRange, FVector& OutTraceEnd) const
{
	if (!Ability) // Server and launching client only
	{
		return;
	}

	// Search around us
	const UCapsuleComponent* CapsuleComponent = InSourceActor && InSourceActor->GetRootComponent() ?
		Cast<UCapsuleComponent>(InSourceActor->GetRootComponent()) : nullptr;

	if (!CapsuleComponent)
	{
		return;
	}

	const float Radius = CapsuleComponent->GetScaledCapsuleRadius();
	const float HalfHeight = CapsuleComponent->GetScaledCapsuleHalfHeight();
	FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(Radius, HalfHeight);

	FHitResult Hit;
	ShapeTrace(Hit, InSourceActor->GetWorld(), CapsuleComponent->GetComponentLocation(),
		TraceChannel, Params, CapsuleShape);

	const bool bUseTraceResult = Hit.bBlockingHit &&
		(FVector::DistSquared(TraceStart, Hit.Location) <= (MaxRange * MaxRange));

	Hit.bBlockingHit &= bUseTraceResult;
}

void UAbilityTask_WaitForPushTargets::UpdatePushOptions(const FPushQuery& PushQuery, const TArray<TScriptInterface<IPusherTarget>>& PushTargets)
{
	TArray<FPushOption> NewOptions;

	for (const TScriptInterface<IPusherTarget>& PushiveTarget : PushTargets)
	{
		TArray<FPushOption> TempOptions;
		FPushOptionBuilder PushBuilder(PushiveTarget, TempOptions);
		PushiveTarget->GatherPushOptions(PushQuery, PushBuilder);

		for (FPushOption& Option : TempOptions)
		{
			const FGameplayAbilitySpec* PushAbilitySpec = nullptr;

			// if there is a handle an a target ability system, we're triggering the ability on the target.
			if (Option.TargetAbilitySystem && Option.TargetPushAbilityHandle.IsValid())
			{
				// Find the spec
				PushAbilitySpec = Option.TargetAbilitySystem->FindAbilitySpecFromHandle(Option.TargetPushAbilityHandle);
			}
			// If there's an Push ability then we're activating it on ourselves.
			else if (Option.PushAbilityToGrant)
			{
				// Find the spec
				PushAbilitySpec = AbilitySystemComponent->FindAbilitySpecFromClass(Option.PushAbilityToGrant);

				if (PushAbilitySpec)
				{
					// update the option
					Option.TargetAbilitySystem = AbilitySystemComponent.Get();
					Option.TargetPushAbilityHandle = PushAbilitySpec->Handle;
				}
			}

			if (PushAbilitySpec)
			{
				// Filter any options that we can't activate right now for whatever reason.
				if (PushAbilitySpec->Ability->CanActivateAbility(PushAbilitySpec->Handle, AbilitySystemComponent->AbilityActorInfo.Get()))
				{
					NewOptions.Add(Option);
				}
			}
		}
	}

	bool bOptionsChanged = false;
	if (NewOptions.Num() == CurrentOptions.Num())
	{
		NewOptions.Sort();

		for (int OptionIndex = 0; OptionIndex < NewOptions.Num(); OptionIndex++)
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

	if (bOptionsChanged)
	{
		CurrentOptions = NewOptions;
		PushObjectsChanged.Broadcast(CurrentOptions);
	}
}