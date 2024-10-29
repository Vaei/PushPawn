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
class UGameplayAbility;
class UPrimitiveComponent;
class AActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPushObjectsChangedEvent, const TArray<FPushOption>&, PushOptions);

/**
 * Waits for push targets to be available.
 * Base class for other scanning tasks.
 */
UCLASS(Abstract)
class PUSHPAWN_API UAbilityTask_PushPawnScan_Base : public UAbilityTask
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FPushObjectsChangedEvent PushObjectsChanged;

	/** GameplayAbility that created us */
	UPROPERTY()
	TObjectPtr<UPushPawn_Scan_Base> PushScanAbility;
	
protected:
	ECollisionChannel TraceChannel;

	TArray<FPushOption> CurrentOptions;

	TMap<FObjectKey, FGameplayAbilitySpecHandle> PushAbilityCache;

protected:
	UAbilityTask_PushPawnScan_Base(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	static void ShapeTrace(FHitResult& OutHitResult, const UWorld* World, const FVector& Center, const FQuat& Rotation,
		const ECollisionChannel ChannelName, const FCollisionQueryParams& Params, const FCollisionShape& Shape);

	void UpdatePushOptions(const FPushQuery& PushQuery, const TArray<TScriptInterface<IPusherTarget>>& PushTargets);

	virtual void OnDestroy(bool bInOwnerFinished) override;
};
