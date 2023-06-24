// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PushStatics.generated.h"

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
};
