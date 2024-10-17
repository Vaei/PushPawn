// Copyright (c) Jared Taylor. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "PushOption.generated.h"

class IPusherTarget;
class UUserWidget;

/**  */
USTRUCT(BlueprintType)
struct FPushOption
{
	GENERATED_BODY()

	FPushOption()
		: PusherTarget(nullptr)
		, PusheeActorLocation(FVector::ZeroVector)
		, PusheeForwardVector(FVector::ForwardVector)
		, PusherActorLocation(FVector::ZeroVector)
	{}
	
public:
	/** The Pusher target, who will do the pushing - this is NOT the ability instigator! */
	UPROPERTY(BlueprintReadWrite, Category=Push)
	TScriptInterface<IPusherTarget> PusherTarget;

	/** This is the instigating pawn that will be pushed, usually a player character */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Push)
	FVector PusheeActorLocation;

	/** Only used when no direction can be obtained to move directly backwards, eg. PusheeActorLocation == PusherActorLocation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Push)
	FVector PusheeForwardVector;

	/** This is the pushing character that will push, usually an AI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Push)
	FVector PusherActorLocation;

	// METHODS OF Push
	//--------------------------------------------------------------

	// 1) Place an ability on the avatar that they can activate when they perform Push.

	/** The ability to grant the avatar when they get near Push objects. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Push)
	TSubclassOf<UGameplayAbility> PushAbilityToGrant;

	// - OR -

	// 2) Allow the object we're interacting with to have its own ability system and Push ability, that we can activate instead.

	/** The ability system on the target that can be used for the TargetPushHandle and sending the event, if needed. */
	UPROPERTY(BlueprintReadOnly, Category=Push)
	TObjectPtr<UAbilitySystemComponent> TargetAbilitySystem = nullptr;

	/** The ability spec to activate on the object for this option. */
	UPROPERTY(BlueprintReadOnly, Category=Push)
	FGameplayAbilitySpecHandle TargetPushAbilityHandle;

	//--------------------------------------------------------------

public:
	FORCEINLINE bool operator==(const FPushOption& Other) const
	{
		return PusherTarget == Other.PusherTarget &&
			PushAbilityToGrant == Other.PushAbilityToGrant&&
			TargetAbilitySystem == Other.TargetAbilitySystem &&
			TargetPushAbilityHandle == Other.TargetPushAbilityHandle &&
			PusheeActorLocation.Equals(Other.PusheeActorLocation) &&
			PusherActorLocation.Equals(Other.PusherActorLocation);
	}

	FORCEINLINE bool operator!=(const FPushOption& Other) const
	{
		return !operator==(Other);
	}

	FORCEINLINE bool operator<(const FPushOption& Other) const
	{
		return PusherTarget.GetInterface() < Other.PusherTarget.GetInterface();
	}
};
