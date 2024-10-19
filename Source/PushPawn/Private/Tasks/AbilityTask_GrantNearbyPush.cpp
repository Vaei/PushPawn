// Copyright (c) Jared Taylor. All Rights Reserved


#include "Tasks/AbilityTask_GrantNearbyPush.h"

#include "AbilitySystemComponent.h"
#include "IPush.h"
#include "PushQuery.h"
#include "PushStatics.h"
#include "Engine/OverlapResult.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AbilityTask_GrantNearbyPush)

void UAbilityTask_GrantNearbyPush::Activate()
{
	SetWaitingOnAvatar();

	ActivateTimer();
}

void UAbilityTask_GrantNearbyPush::OnDestroy(bool bInOwnerFinished)
{
	Super::OnDestroy(bInOwnerFinished);

	if (TimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	}
}

void UAbilityTask_GrantNearbyPush::ActivateTimer()
{
	// If we don't have an ability, we can't do anything
	if (!Ability)
	{
		return;
	}

	// Determine the scan rate
	float ScanRate = ScanParams.ScanRate;
	if (!IsWaitingOnAvatar() && GetAvatarActor())
	{
		if (APawn* Pawn = Cast<APawn>(GetAvatarActor()))
		{
			// Get the scan rate from the pawn
			ScanRate = UPushStatics::GetPushPawnScanRate(Pawn, ScanParams);
		}
	}
	
	// Set the timer to trigger the trace
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ThisClass::QueryPushes, ScanRate, false);
}

void UAbilityTask_GrantNearbyPush::QueryPushes()
{
	const UWorld* World = GetWorld();
	AActor* AvatarActor = GetAvatarActor();
	
	if (World && AvatarActor)
	{
		// Setup the query parameters
		FCollisionQueryParams Params(SCENE_QUERY_STAT(UAbilityTask_GrantNearbyPush), false);
		Params.AddIgnoredActor(AvatarActor);

		// Determine the scan range
		APawn* AvatarPawn = Cast<APawn>(AvatarActor);
		const float ScanRange = UPushStatics::GetPushPawnScanRange(AvatarPawn, BaseScanRange, ScanParams);

		// Perform the overlap
		TArray<FOverlapResult> OverlapResults;
		World->OverlapMultiByChannel(OverlapResults, AvatarActor->GetActorLocation(), FQuat::Identity, ScanParams.TraceChannel, FCollisionShape::MakeSphere(ScanRange), Params);

		if (OverlapResults.Num() > 0)
		{
			// Gather all the push targets
			TArray<TScriptInterface<IPusherTarget>> PushTargets;
			UPushStatics::AppendPushTargetsFromOverlapResults(OverlapResults, PushTargets);

			// Setup the push query
			FPushQuery PushQuery;
			PushQuery.RequestingAvatar = AvatarActor;

			// Gather all the push options
			TArray<FPushOption> Options;
			for (const TScriptInterface<IPusherTarget>& PushTarget : PushTargets)
			{
				FPushOptionBuilder PushBuilder(PushTarget, Options);
				PushTarget->GatherPushOptions(PushQuery, PushBuilder);
			}

			// Check if any of the options need to grant the ability to the user before they can be used.
			for (FPushOption& Option : Options)
			{
				if (Option.PushAbilityToGrant)
				{
					// Grant the ability to the GAS, otherwise it won't be able to do whatever the Push is.
					FObjectKey ObjectKey { Option.PushAbilityToGrant };
					if (!PushAbilityCache.Find(ObjectKey))
					{
						FGameplayAbilitySpec Spec(Option.PushAbilityToGrant, 1, INDEX_NONE, this);
						FGameplayAbilitySpecHandle Handle = AbilitySystemComponent->GiveAbility(Spec);
						PushAbilityCache.Add(ObjectKey, Handle);
					}
				}
			}
		}
	}

	// Loop the timer
	ActivateTimer();
}

UAbilityTask_GrantNearbyPush* UAbilityTask_GrantNearbyPush::GrantAbilitiesForNearbyPushers(
	UGameplayAbility* OwningAbility, const FPushPawnScanParams& InScanParams, float InBaseScanRange)
{
	UAbilityTask_GrantNearbyPush* MyObj = NewAbilityTask<UAbilityTask_GrantNearbyPush>(OwningAbility);
	MyObj->ScanParams = InScanParams;
	MyObj->BaseScanRange = InBaseScanRange;
	return MyObj;
}
