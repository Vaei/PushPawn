// Copyright (c) Jared Taylor. All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Engine/CollisionProfile.h"
#include "PushOption.h"
#include "PushQuery.h"
#include "IPush.h"
#include "AbilityTask_PushPawnScan_Base.generated.h"

class UPushPawn_Scan_Base;
class AActor;
class UPrimitiveComponent;
class UGameplayAbility;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPushObjectsChangedEvent, const TArray<FPushOption>&, PushOptions);

/**
 * Waits for push targets to be available.
 * Base class for other scanning tasks.
 */
UCLASS(Abstract)
class PUSHPAWN_API UAbilityTask_PushPawnScan_Base : public UAbilityTask
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FPushObjectsChangedEvent PushObjectsChanged;

	/** GameplayAbility that created us */
	UPROPERTY()
	TObjectPtr<UPushPawn_Scan_Base> PushScanAbility;
	
protected:
	ECollisionChannel TraceChannel;

	TArray<FPushOption> CurrentOptions;

protected:
	static void ShapeTrace(FHitResult& OutHitResult, const UWorld* World, const FVector& Center, const ECollisionChannel ChannelName, const FCollisionQueryParams& Params, const FCollisionShape& Shape);

	void UpdatePushOptions(const FPushQuery& PushQuery, const TArray<TScriptInterface<IPusherTarget>>& PushTargets);

	virtual void OnDestroy(bool bInOwnerFinished) override;
};
