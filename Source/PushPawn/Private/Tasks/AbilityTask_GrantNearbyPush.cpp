// Copyright (c) Jared Taylor. All Rights Reserved


#include "Tasks/AbilityTask_GrantNearbyPush.h"

#include "AbilitySystemComponent.h"
#include "IPush.h"
#include "PushQuery.h"
#include "PushStatics.h"

void UAbilityTask_GrantNearbyPush::Activate()
{
	SetWaitingOnAvatar();

	GetWorld()->GetTimerManager().SetTimer(QueryTimerHandle, this, &ThisClass::QueryPushs, PushScanRate, true);
}

void UAbilityTask_GrantNearbyPush::OnDestroy(bool bInOwnerFinished)
{
	Super::OnDestroy(bInOwnerFinished);

	GetWorld()->GetTimerManager().ClearTimer(QueryTimerHandle);
}

void UAbilityTask_GrantNearbyPush::QueryPushs()
{
	const UWorld* World = GetWorld();
	AActor* ActorOwner = GetAvatarActor();
	
	if (World && ActorOwner)
	{
		FCollisionQueryParams Params(SCENE_QUERY_STAT(UAbilityTask_GrantNearbyPush), false);
		Params.AddIgnoredActor(ActorOwner);
		
		TArray<FOverlapResult> OverlapResults;
		World->OverlapMultiByChannel(OUT OverlapResults, ActorOwner->GetActorLocation(), FQuat::Identity, TraceChannel, FCollisionShape::MakeSphere(PushScanRange), Params);

		if (OverlapResults.Num() > 0)
		{
			TArray<TScriptInterface<IPusherTarget>> PushTargets;
			UPushStatics::AppendPushTargetsFromOverlapResults(OverlapResults, OUT PushTargets);
			
			FPushQuery PushQuery;
			PushQuery.RequestingAvatar = ActorOwner;

			TArray<FPushOption> Options;
			for (const TScriptInterface<IPusherTarget>& InteractiveTarget : PushTargets)
			{
				FPushOptionBuilder PushBuilder(InteractiveTarget, Options);
				InteractiveTarget->GatherPushOptions(PushQuery, PushBuilder);
			}

			// Check if any of the options need to grant the ability to the user before they can be used.
			for (FPushOption& Option : Options)
			{
				if (Option.PushAbilityToGrant)
				{
					// Grant the ability to the GAS, otherwise it won't be able to do whatever the Push is.
					FObjectKey ObjectKey(Option.PushAbilityToGrant);
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
}

UAbilityTask_GrantNearbyPush* UAbilityTask_GrantNearbyPush::GrantAbilitiesForNearbyPushers(
	UGameplayAbility* OwningAbility, ECollisionChannel TraceChannel, float PushScanRange, float PushScanRate)
{
	UAbilityTask_GrantNearbyPush* MyObj = NewAbilityTask<UAbilityTask_GrantNearbyPush>(OwningAbility);
	MyObj->TraceChannel = TraceChannel;
	MyObj->PushScanRange = PushScanRange;
	MyObj->PushScanRate = PushScanRate;
	return MyObj;
}
