// Copyright Epic Games, Inc. All Rights Reserved.

#include "Tasks/AbilityTask_WaitForPushTargets_CapsuleTrace.h"
#include "GameFramework/Actor.h"
#include "IPush.h"
#include "PushStatics.h"
#include "PushQuery.h"
#include "AbilitySystemComponent.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AbilityTask_WaitForPushTargets_CapsuleTrace)

UAbilityTask_WaitForPushTargets_CapsuleTrace::UAbilityTask_WaitForPushTargets_CapsuleTrace(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UAbilityTask_WaitForPushTargets_CapsuleTrace* UAbilityTask_WaitForPushTargets_CapsuleTrace::WaitForPushTargets_CapsuleTrace(UGameplayAbility* OwningAbility, FPushQuery PushQuery, ECollisionChannel TraceChannel, FGameplayAbilityTargetingLocationInfo StartLocation, float PushScanRange, float PushScanRate, bool bShowDebug)
{
	UAbilityTask_WaitForPushTargets_CapsuleTrace* MyObj = NewAbilityTask<UAbilityTask_WaitForPushTargets_CapsuleTrace>(OwningAbility);
	MyObj->PushScanRange = PushScanRange;
	MyObj->PushScanRate = PushScanRate;
	MyObj->StartLocation = StartLocation;
	MyObj->PushQuery = PushQuery;
	MyObj->TraceChannel = TraceChannel;
	MyObj->bShowDebug = bShowDebug;

	return MyObj;
}

void UAbilityTask_WaitForPushTargets_CapsuleTrace::Activate()
{
	SetWaitingOnAvatar();

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ThisClass::PerformTrace, PushScanRate, true);
}

void UAbilityTask_WaitForPushTargets_CapsuleTrace::OnDestroy(bool AbilityEnded)
{
	Super::OnDestroy(AbilityEnded);

	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
}

void UAbilityTask_WaitForPushTargets_CapsuleTrace::PerformTrace()
{
	AActor* AvatarActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();
	if (!AvatarActor)
	{
		return;
	}

	UWorld* World = GetWorld();

	// Search around us
	const UCapsuleComponent* CapsuleComponent = AvatarActor && AvatarActor->GetRootComponent() ?
		Cast<UCapsuleComponent>(AvatarActor->GetRootComponent()) : nullptr;

	if (!CapsuleComponent)
	{
		return;
	}

	const float Radius = CapsuleComponent->GetScaledCapsuleRadius();
	const float HalfHeight = CapsuleComponent->GetScaledCapsuleHalfHeight();
	FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(Radius, HalfHeight);

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(AvatarActor);
	
	constexpr bool bTraceComplex = false;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(UAbilityTask_WaitForInteractableTargets_SingleLineTrace), bTraceComplex);
	Params.AddIgnoredActors(ActorsToIgnore);

	FVector TraceStart = StartLocation.GetTargetingTransform().GetLocation();

	FHitResult Hit;
	ShapeTrace(Hit, GetWorld(), TraceStart, TraceChannel, Params, CapsuleShape);

	TArray<TScriptInterface<IPusherTarget>> PushTargets;
	UPushStatics::AppendPushTargetsFromHitResult(Hit, PushTargets);

	UpdatePushOptions(PushQuery, PushTargets);

#if ENABLE_DRAW_DEBUG
	if (bShowDebug)
	{
		FColor DebugColor = Hit.bBlockingHit ? FColor::Red : FColor::Green;
		DrawDebugCapsule(World, TraceStart, HalfHeight, Radius, FQuat::Identity, DebugColor, false, PushScanRate);
		if (Hit.bBlockingHit)
		{
			DrawDebugSphere(World, Hit.Location, 5, 16, DebugColor, false, PushScanRate);
		}
	}
#endif // ENABLE_DRAW_DEBUG
}
