// Copyright (c) Jared Taylor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "GameFramework/RootMotionSource.h"
#include "AbilityTask_PushPawnForce.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FApplyPushPawnForceDelegate);

/**
 * Apply a constant root motion force
 * This is much more lightweight than ApplyRootMotionConstantForce
 *
 * It does not replicate anything
 *	Because it is local predicted and originates from CMC, it is not necessary to replicate
 */
UCLASS()
class PUSHPAWN_API UAbilityTask_PushPawnForce : public UAbilityTask
{
	GENERATED_BODY()

public:
	UAbilityTask_PushPawnForce(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Apply force to character's movement */
	static UAbilityTask_PushPawnForce* ApplyPushPawnForce
	(
		UGameplayAbility* OwningAbility,
		const FVector& WorldDirection,
		float Strength,
		float Duration,
		bool bIsAdditive = true,
		bool bEnableGravity = true,
		UCurveFloat* StrengthOverTime = nullptr
	);

	/** Apply force to character's movement */
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta=(DisplayName="Apply Push Pawn Force", HidePin="OwningAbility", DefaultToSelf="OwningAbility", BlueprintInternalUseOnly="TRUE"))
	static UAbilityTask_PushPawnForce* K2_ApplyPushPawnForce
	(
		UGameplayAbility* OwningAbility, 
		FVector WorldDirection,
		float Strength,
		float Duration,
		bool bIsAdditive = true,
		bool bEnableGravity = true,
		UCurveFloat* StrengthOverTime = nullptr
	);

	virtual void InitSimulatedTask(UGameplayTasksComponent& InGameplayTasksComponent) override;
	
	virtual void SharedInitAndApply();
	
protected:
	bool HasTimedOut() const;

	virtual void OnDestroy(bool bInOwnerFinished) override;

public:
	UPROPERTY(BlueprintAssignable)
	FApplyPushPawnForceDelegate OnFinish;

	virtual void PreDestroyFromReplication() override;
	
protected:
	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> MovementComponent; 
	
protected:
	UPROPERTY()
	FVector WorldDirection;

	UPROPERTY()
	float Strength;

	UPROPERTY()
	float Duration;

	UPROPERTY()
	TObjectPtr<UCurveFloat> StrengthOverTime;

	UPROPERTY()
	bool bIsAdditive;

	UPROPERTY()
	bool bEnableGravity;
	
	uint16 RootMotionSourceID;

public:
	FTimerHandle FinishTimerHandle;
	
	bool bIsFinished;
};
