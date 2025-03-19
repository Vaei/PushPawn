// Copyright (c) Jared Taylor. All Rights Reserved.


#include "Tasks/AbilityTask_PushPawnForce.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemLog.h"

#include "TimerManager.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AbilityTask_PushPawnForce)

static const FName PushPawnForceName = TEXT("PushPawnForce");

UAbilityTask_PushPawnForce::UAbilityTask_PushPawnForce(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTickingTask = false;
	bSimulatedTask = true;

	Priority = 2;
	WorldDirection = FVector::ZeroVector;
	Strength = 0.f;
	Duration = 0.f;
	StrengthOverTime = nullptr;
	bIsAdditive = true;
	bEnableGravity = false;

	RootMotionSourceID = (uint16)ERootMotionSourceID::Invalid;
	bIsFinished = false;
}

UAbilityTask_PushPawnForce* UAbilityTask_PushPawnForce::ApplyPushPawnForce(UGameplayAbility* OwningAbility,
	const FVector& WorldDirection, float Strength, float Duration, bool bIsAdditive, bool bEnableGravity,
	UCurveFloat* StrengthOverTime)
{
	UAbilitySystemGlobals::NonShipping_ApplyGlobalAbilityScaler_Duration(Duration);

	UAbilityTask_PushPawnForce* MyTask = NewAbilityTask<UAbilityTask_PushPawnForce>(OwningAbility, PushPawnForceName);

	MyTask->WorldDirection = WorldDirection.GetSafeNormal();
	MyTask->Strength = Strength;
	MyTask->Duration = Duration;
	MyTask->bIsAdditive = bIsAdditive;
	MyTask->bEnableGravity = bEnableGravity;
	MyTask->StrengthOverTime = StrengthOverTime;

	// Don't forget to call SharedInitAndApply() after creating this task - it is not performed automatically,
	//	so you can bind to the OnTaskEnded delegate before calling Activate() to ensure you don't miss the event
	
	return MyTask;
}

UAbilityTask_PushPawnForce* UAbilityTask_PushPawnForce::K2_ApplyPushPawnForce(
	UGameplayAbility* OwningAbility,
	FVector WorldDirection,
	float Strength,
	float Duration,
	bool bIsAdditive,
	bool bEnableGravity,
	UCurveFloat* StrengthOverTime)
{
	UAbilitySystemGlobals::NonShipping_ApplyGlobalAbilityScaler_Duration(Duration);

	UAbilityTask_PushPawnForce* MyTask = NewAbilityTask<UAbilityTask_PushPawnForce>(OwningAbility, PushPawnForceName);

	MyTask->WorldDirection = WorldDirection.GetSafeNormal();
	MyTask->Strength = Strength;
	MyTask->Duration = Duration;
	MyTask->bIsAdditive = bIsAdditive;
	MyTask->bEnableGravity = bEnableGravity;
	MyTask->StrengthOverTime = StrengthOverTime;
	MyTask->SharedInitAndApply();

	return MyTask;
}

void UAbilityTask_PushPawnForce::InitSimulatedTask(UGameplayTasksComponent& InGameplayTasksComponent)
{
	Super::InitSimulatedTask(InGameplayTasksComponent);
	
	SharedInitAndApply();
}

void UAbilityTask_PushPawnForce::SharedInitAndApply()
{
	const UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (ASC && ASC->AbilityActorInfo->MovementComponent.IsValid())
	{
		MovementComponent = Cast<UCharacterMovementComponent>(ASC->AbilityActorInfo->MovementComponent.Get());

		if (MovementComponent)
		{
			const TSharedPtr<FRootMotionSource_ConstantForce> ConstantForce = MakeShared<FRootMotionSource_ConstantForce>();
			ConstantForce->InstanceName = TEXT("PushPawnForce");
			ConstantForce->AccumulateMode = bIsAdditive ? ERootMotionAccumulateMode::Additive : ERootMotionAccumulateMode::Override;
			ConstantForce->Priority = 2;  // Low priority for pushing
			ConstantForce->Force = WorldDirection * Strength;
			ConstantForce->Duration = Duration;
			ConstantForce->StrengthOverTime = StrengthOverTime;
			ConstantForce->FinishVelocityParams.Mode = ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity;
			if (bEnableGravity)
			{
				ConstantForce->Settings.SetFlag(ERootMotionSourceSettingsFlags::IgnoreZAccumulate);
			}
			RootMotionSourceID = MovementComponent->ApplyRootMotionSource(ConstantForce);
			if (RootMotionSourceID != (uint16)ERootMotionSourceID::Invalid)
			{
				const FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &ThisClass::EndTask);
				GetWorld()->GetTimerManager().SetTimer(FinishTimerHandle, TimerDelegate, Duration, false);
			}
			else
			{
				ABILITY_LOG(Warning, TEXT("UAbilityTask_PushPawnForce failed to apply root motion source; Task Instance Name %s."), *InstanceName.ToString());
				EndTask();
			}
		}
	}
	else
	{
		ABILITY_LOG(Warning, TEXT("UAbilityTask_PushPawnForce called in Ability %s with null MovementComponent; Task Instance Name %s."), 
			Ability ? *Ability->GetName() : TEXT("NULL"), 
			*InstanceName.ToString());
		EndTask();
	}
}

bool UAbilityTask_PushPawnForce::HasTimedOut() const
{
	const TSharedPtr<FRootMotionSource> RMS = (MovementComponent ? MovementComponent->GetRootMotionSourceByID(RootMotionSourceID) : nullptr);
	if (!RMS.IsValid())
	{
		return true;
	}

	return RMS->Status.HasFlag(ERootMotionSourceStatusFlags::Finished);
}

void UAbilityTask_PushPawnForce::OnDestroy(bool bInOwnerFinished)
{
	if (MovementComponent)
	{
		MovementComponent->RemoveRootMotionSourceByID(RootMotionSourceID);
	}

	// Used to end an ability
	bIsFinished = true;
	OnFinish.Broadcast();
	
	Super::OnDestroy(bInOwnerFinished);
}

void UAbilityTask_PushPawnForce::PreDestroyFromReplication()
{
	EndTask();
}
