// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_PushPawnSync.generated.h"

class UAbilityTask_PushPawnSync;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPushPawnNetworkSyncDelegate, UAbilityTask_PushPawnSync*, SyncPoint);

/**
 * Task for providing a generic sync point for client server (one can wait for a signal from the other)
 * Identical to UAbilityTask_SyncPushPawn, except the delegate provides the node that broadcasted so it can be removed from a TArray
 * Only performs OnlyServerWait
 */
UCLASS()
class PUSHPAWN_API UAbilityTask_PushPawnSync : public UAbilityTask
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FPushPawnNetworkSyncDelegate OnSync;

	UAbilityTask_PushPawnSync(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UFUNCTION()
	void OnSignalCallback();

	virtual void Activate() override;

	/**
	 *	
	 *	Synchronize execution flow between Client and Server. Depending on SyncType, the Client and Server will wait for the other to reach this node or another WaitNetSync node in the ability before continuing execution.  
	 *	
	 *	BothWait - Both Client and Server will wait until the other reaches the node. (Whoever gets their first, waits for the other before continueing).
	 *	OnlyServerWait - Only server will wait for the client signal. Client will signal and immediately continue without waiting to hear from Server.
	 *	OnlyClientWait - Only client will wait for the server signal. Server will signal and immediately continue without waiting to hear from Client.
	 *	
	 *	Note that this is "ability instance wide". These sync points never affect sync points in other abilities. 
	 *	
	 *	In most cases you will have both client and server execution paths connected to the same WaitNetSync node. However it is possible to use separate nodes
	 *	for cleanliness of the graph. The "signal" is "ability instance wide".
	 *	
	 */
	static UAbilityTask_PushPawnSync* WaitNetSync(UGameplayAbility* OwningAbility);

protected:

	void SyncFinished();

	/** The event we replicate */
	EAbilityGenericReplicatedEvent::Type ReplicatedEventToListenFor;
};
