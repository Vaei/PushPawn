// Copyright (c) Jared Taylor. All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Engine/CollisionProfile.h"
#include "PushOption.h"
#include "PushQuery.h"
#include "IPush.h"
#include "AbilityTask_WaitForPushTargets.generated.h"

class AActor;
class UPrimitiveComponent;
class UGameplayAbility;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPushObjectsChangedEvent, const TArray<FPushOption>&, PushOptions);

UCLASS(Abstract)
class PUSHPAWN_API UAbilityTask_WaitForPushTargets : public UAbilityTask
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FPushObjectsChangedEvent PushObjectsChanged;

protected:
	ECollisionChannel TraceChannel;

	TArray<FPushOption> CurrentOptions;

protected:
	static void ShapeTrace(FHitResult& OutHitResult, const UWorld* World, const FVector& Center, const ECollisionChannel ChannelName, const FCollisionQueryParams& Params, const FCollisionShape& Shape);

	void DetectNearbyTargets(const AActor* InSourceActor, FCollisionQueryParams Params, const FVector& TraceStart, float MaxRange, FVector& OutTraceEnd) const;

	void UpdatePushOptions(const FPushQuery& PushQuery, const TArray<TScriptInterface<IPusherTarget>>& PushTargets);
};
