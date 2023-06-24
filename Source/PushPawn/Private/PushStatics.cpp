// Copyright Epic Games, Inc. All Rights Reserved.

#include "PushStatics.h"
#include "IPush.h"

UPushStatics::UPushStatics()
	: Super(FObjectInitializer::Get())
{
}

AActor* UPushStatics::GetActorFromPushTarget(TScriptInterface<IPusherTarget> PushTarget)
{
	if (UObject* Object = PushTarget.GetObject())
	{
		if (AActor* Actor = Cast<AActor>(Object))
		{
			return Actor;
		}
		else if (const UActorComponent* ActorComponent = Cast<UActorComponent>(Object))
		{
			return ActorComponent->GetOwner();
		}
		else
		{
			unimplemented();
		}
	}

	return nullptr;
}

void UPushStatics::GetPushTargetsFromActor(AActor* Actor, TArray<TScriptInterface<IPusherTarget>>& OutPushTargets)
{
	// If the actor is directly Pusher, return that.
	const TScriptInterface<IPusherTarget> PushActor(Actor);
	if (PushActor)
	{
		OutPushTargets.Add(PushActor);
	}

	// If the actor isn't Push, it might have a component that has a Push interface.
	TArray<UActorComponent*> PushComponents = Actor ? Actor->GetComponentsByInterface(UPusheeInstigator::StaticClass()) : TArray<UActorComponent*>();
	for (UActorComponent* PushComponent : PushComponents)
	{
		OutPushTargets.Add(TScriptInterface<IPusherTarget>(PushComponent));
	}
}

void UPushStatics::AppendPushTargetsFromOverlapResults(const TArray<FOverlapResult>& OverlapResults, TArray<TScriptInterface<IPusherTarget>>& OutPushTargets)
{
	for (const FOverlapResult& Overlap : OverlapResults)
	{
		TScriptInterface<IPusherTarget> PushActor(Overlap.GetActor());
		if (PushActor)
		{
			OutPushTargets.AddUnique(PushActor);
		}

		TScriptInterface<IPusherTarget> PushComponent(Overlap.GetComponent());
		if (PushComponent)
		{
			OutPushTargets.AddUnique(PushComponent);
		}
	}
}

void UPushStatics::AppendPushTargetsFromHitResult(const FHitResult& HitResult, TArray<TScriptInterface<IPusherTarget>>& OutPushTargets)
{
	const TScriptInterface<IPusherTarget> PushActor(HitResult.GetActor());
	if (PushActor)
	{
		OutPushTargets.AddUnique(PushActor);
	}

	const TScriptInterface<IPusherTarget> PushComponent(HitResult.GetComponent());
	if (PushComponent)
	{
		OutPushTargets.AddUnique(PushComponent);
	}
}