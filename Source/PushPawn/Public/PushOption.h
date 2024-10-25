// Copyright (c) Jared Taylor. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "PushOption.generated.h"

class IPusherTarget;
class UUserWidget;

/**
 * Information required to push a pawn.
 */
USTRUCT(BlueprintType)
struct PUSHPAWN_API FPushOption
{
	GENERATED_BODY()

	FPushOption()
		: PusherTarget(nullptr)
		, PusheeActorLocation(FVector::ZeroVector)
		, PusheeForwardVector(FVector::ForwardVector)
		, PusherActorLocation(FVector::ZeroVector)
	{}
	
	/** The Pusher target, who will do the pushing - this is not the ability instigator! */
	UPROPERTY(BlueprintReadWrite, Category=PushPawn)
	TScriptInterface<IPusherTarget> PusherTarget;

	/** This is the instigating pawn that will be pushed, usually a player character */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=PushPawn)
	FVector PusheeActorLocation;

	/** Only used when no direction can be obtained to move directly backwards, e.g. PusheeActorLocation == PusherActorLocation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=PushPawn)
	FVector PusheeForwardVector;

	/** This is the pawn that will push someone else */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=PushPawn)
	FVector PusherActorLocation;

	/** The ability to grant the avatar when they get pushed */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=PushPawn)
	TSubclassOf<UGameplayAbility> PushAbilityToGrant;

	/** The ability system on the target that can be used for sending the event */
	UPROPERTY(BlueprintReadOnly, Category=PushPawn)
	TObjectPtr<UAbilitySystemComponent> TargetAbilitySystem = nullptr;

	/** The ability spec to activate on the object for this option. */
	UPROPERTY(BlueprintReadOnly, Category=PushPawn)
	FGameplayAbilitySpecHandle TargetPushAbilityHandle;

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
