// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PushTypes.h"
#include "PushStatics.generated.h"

class IPusherTarget;

/**  */
UCLASS()
class PUSHPAWN_API UPushStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UPushStatics();

public:
	UFUNCTION(BlueprintCallable, Category=Push)
	static AActor* GetActorFromPushTarget(TScriptInterface<IPusherTarget> PushTarget);

	UFUNCTION(BlueprintCallable, Category=Push)
	static void GetPushTargetsFromActor(AActor* Actor, TArray<TScriptInterface<IPusherTarget>>& OutPushTargets);

	static void AppendPushTargetsFromOverlapResults(const TArray<FOverlapResult>& OverlapResults, TArray<TScriptInterface<IPusherTarget>>& OutPushTargets);
	static void AppendPushTargetsFromHitResult(const FHitResult& HitResult, TArray<TScriptInterface<IPusherTarget>>& OutPushTargets);

public:
	/** 
	 * Returns degree of the angle between Direction and Rotation forward vector
	 * The range of return will be from [-180, 180].
	 * @param	Direction		The direction relative to BaseRotation
	 * @param	BaseRotation	The base rotation, e.g. of a pawn
	 */
	static float CalculatePushDirection(const FVector& Direction, const FRotator& BaseRotation);

	UFUNCTION(BlueprintCallable, Category=Push, meta=(ExpandEnumAsExecs="ValidPushDirection"))
	static EPushCardinal GetPushDirection(const AActor* FromActor, const AActor* ToActor, EValidPushDirection& ValidPushDirection);
};
