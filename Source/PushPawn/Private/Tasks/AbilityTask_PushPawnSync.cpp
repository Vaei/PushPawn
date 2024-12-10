// Copyright Epic Games, Inc. All Rights Reserved.

#include "Tasks/AbilityTask_PushPawnSync.h"
#include "AbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AbilityTask_PushPawnSync)

UAbilityTask_PushPawnSync::UAbilityTask_PushPawnSync(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ReplicatedEventToListenFor = EAbilityGenericReplicatedEvent::MAX;
}

void UAbilityTask_PushPawnSync::OnSignalCallback()
{
	if (AbilitySystemComponent.IsValid())
	{
		AbilitySystemComponent->ConsumeGenericReplicatedEvent(ReplicatedEventToListenFor, GetAbilitySpecHandle(), GetActivationPredictionKey());
	}
	SyncFinished();
}

UAbilityTask_PushPawnSync* UAbilityTask_PushPawnSync::WaitNetSync(class UGameplayAbility* OwningAbility)
{
	UAbilityTask_PushPawnSync* MyObj = NewAbilityTask<UAbilityTask_PushPawnSync>(OwningAbility);
	return MyObj;
}

void UAbilityTask_PushPawnSync::Activate()
{
	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get(), IsPredictingClient());

	if (AbilitySystemComponent.IsValid())
	{
		if (IsPredictingClient())
		{
			// As long as the server is waiting (!= OnlyClientWait), send the Server and RPC for this signal
			AbilitySystemComponent->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::GenericSignalFromClient, GetAbilitySpecHandle(), GetActivationPredictionKey(), AbilitySystemComponent->ScopedPredictionKey);
		}
		else if (IsForRemoteClient())
		{
			// As long as we are waiting (!= OnlyClientWait), listen for the GenericSignalFromClient event
			ReplicatedEventToListenFor = EAbilityGenericReplicatedEvent::GenericSignalFromClient;
		}

		if (ReplicatedEventToListenFor != EAbilityGenericReplicatedEvent::MAX)
		{
			CallOrAddReplicatedDelegate(ReplicatedEventToListenFor, FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &UAbilityTask_PushPawnSync::OnSignalCallback));
		}
		else
		{
			// We aren't waiting for a replicated event, so the sync is complete.
			SyncFinished();
		}
	}
}

void UAbilityTask_PushPawnSync::SyncFinished()
{
	if (IsValid(this))
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnSync.Broadcast(this);
		}
		EndTask();
	}
}

