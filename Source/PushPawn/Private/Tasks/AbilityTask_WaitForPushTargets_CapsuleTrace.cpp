// Copyright (c) Jared Taylor. All Rights Reserved

#include "Tasks/AbilityTask_WaitForPushTargets_CapsuleTrace.h"
#include "GameFramework/Actor.h"
#include "IPush.h"
#include "PushStatics.h"
#include "PushQuery.h"
#include "AbilitySystemComponent.h"
#include "Abilities/PushPawn_Scan_Base.h"
#include "Abilities/Tasks/AbilityTask_NetworkSyncPoint.h"
#include "Components/CapsuleComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AbilityTask_WaitForPushTargets_CapsuleTrace)

namespace FPushPawn
{
#if ENABLE_DRAW_DEBUG
	static int32 PushPawnScanDebugDraw = 0;
	FAutoConsoleVariableRef CVarPushPawnScanDebugDraw(
		TEXT("p.PushPawn.Scan.Debug.Draw"),
		PushPawnScanDebugDraw,
		TEXT("Optionally draw debug for PushPawn Scan.\n")
		TEXT("0: Disable, 1: Enable"),
		ECVF_Default);
#endif

#if !UE_BUILD_SHIPPING
	static int32 PushPawnPrintNetSync = 0;
	FAutoConsoleVariableRef CVarPushPawnPrintNetSync(
		TEXT("p.PushPawn.PrintNetSync"),
		PushPawnPrintNetSync,
		TEXT("Optionally print to screen when PushPawn performs a NetSync.\n")
		TEXT("0: Disable, 1: Enable for Authority & Local Client, 2: Enable for Authority only, 3: Enable for Local Client only"),
		ECVF_Default);

	static int32 PushPawnPrintScanPaused = 0;
	FAutoConsoleVariableRef CVarPushPawnPrintScanPaused(
		TEXT("p.PushPawn.PrintScanPaused"),
		PushPawnPrintScanPaused,
		TEXT("Optionally print to screen when PushPawn scan is paused or resumed.\n")
		TEXT("0: Disable, 1: Enable for Authority & Local Client, 2: Enable for Authority only, 3: Enable for Local Client only"),
		ECVF_Default);
#endif
}

UAbilityTask_WaitForPushTargets_CapsuleTrace* UAbilityTask_WaitForPushTargets_CapsuleTrace::WaitForPushTargets_CapsuleTrace(
	UGameplayAbility* OwningAbility,
	FPushQuery PushQuery,
	FGameplayAbilityTargetingLocationInfo StartLocation, const FPushPawnScanParams& ScanParams, float ActivationFailureDelay
)
{
	UAbilityTask_WaitForPushTargets_CapsuleTrace* MyObj = NewAbilityTask<UAbilityTask_WaitForPushTargets_CapsuleTrace>(OwningAbility);
	MyObj->ScanParams = ScanParams;
	MyObj->StartLocation = StartLocation;
	MyObj->PushQuery = PushQuery;
	MyObj->ActivationFailureDelay = ActivationFailureDelay;

	return MyObj;
}

void UAbilityTask_WaitForPushTargets_CapsuleTrace::OnNetSync()
{
	ActivateTimer();
}

void UAbilityTask_WaitForPushTargets_CapsuleTrace::ActivateTimer(EPushPawnPauseType PauseType)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UAbilityTask_WaitForPushTargets_CapsuleTrace::ActivateTimer);

	// Wait for net sync if necessary, this prevents the ability from activating until the server has caught up
	if (PushScanAbility && PushScanAbility->ShouldWaitForNetSync())
	{
		// If we don't have an avatar actor, we can't wait for net sync
		if (!GetAvatarActor())
		{
			ActivateTimer(EPushPawnPauseType::ActivationFailed);
			return;
		}

		// Consumes the wait for net sync
		PushScanAbility->ConsumeWaitForNetSync();

#if !UE_BUILD_SHIPPING
		// Print to screen if desired
		if (FPushPawn::PushPawnPrintNetSync > 0)
		{
			const bool bPrintServer = FPushPawn::PushPawnPrintNetSync == 1 || FPushPawn::PushPawnPrintNetSync == 2;
			const bool bPrintClient = FPushPawn::PushPawnPrintNetSync == 1 || FPushPawn::PushPawnPrintNetSync == 3;
			const bool bIsServer = GetAvatarActor()->HasAuthority();
			const bool bIsLocalClient = GetAvatarActor()->GetLocalRole() == ROLE_AutonomousProxy;
			if ((bPrintServer && bIsServer) || (bPrintClient && bIsLocalClient))
			{
				if (GEngine)
				{
					const FString NetRoleString = bIsServer ? TEXT("Server") : TEXT("Client");
					GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::Printf(TEXT("PushPawn: %s NetSync occurred"), *NetRoleString));
				}
			}
		}
#endif
		
		UAbilityTask_NetworkSyncPoint* WaitNetSync = UAbilityTask_NetworkSyncPoint::WaitNetSync(Ability, EAbilityTaskNetSyncType::OnlyServerWait);
		WaitNetSync->OnSync.AddDynamic(this, &ThisClass::OnNetSync);
		WaitNetSync->ReadyForActivation();
		return;
	}

	if (!OnPushPawnScanPauseStateChangedDelegate)
	{
		IPusheeInstigator* Pushee = GetAvatarActor() ? Cast<IPusheeInstigator>(GetAvatarActor()) : nullptr;
		OnPushPawnScanPauseStateChangedDelegate = Pushee->GetPushPawnScanPausedDelegate();
		if (OnPushPawnScanPauseStateChangedDelegate)
		{
			OnPushPawnScanPauseStateChangedDelegate->BindUObject(this, &ThisClass::OnScanPaused);
		}
	}
	
	// Lambda to bind to the timer, used for conditional delays
	auto PendingTimer = [this]()
	{
		if (Ability)
		{
			float ScanRate = ScanParams.ScanRate;
			if (!IsWaitingOnAvatar() && GetAvatarActor())
			{
				if (APawn* Pawn = Cast<APawn>(GetAvatarActor()))
				{
					ScanRate = UPushStatics::GetPushPawnScanRate(Pawn, ScanParams);
				}
			}
			this->CurrentScanRate = ScanRate;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ThisClass::PerformTrace, ScanRate, false);	
		}
	};

	if (PauseType == EPushPawnPauseType::NotPaused)
	{
		// No pause, just set the timer
    	PendingTimer();
	}
	else  // Delay the timer if we're paused
	{
		// Bind the delegate
		FTimerDelegate Delegate;
		Delegate.BindLambda(PendingTimer);

		// Set the timer
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, Delegate, ActivationFailureDelay, false);
	}
}

void UAbilityTask_WaitForPushTargets_CapsuleTrace::Activate()
{
	SetWaitingOnAvatar();

	ActivateTimer();
}

void UAbilityTask_WaitForPushTargets_CapsuleTrace::OnDestroy(bool bInOwnerFinished)
{
	// Clear the timer
	if (TimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	}

	// Unbind the delegate
	if (OnPushPawnScanPauseStateChangedDelegate)
	{
		OnPushPawnScanPauseStateChangedDelegate->Unbind();
		OnPushPawnScanPauseStateChangedDelegate = nullptr;
	}
	Super::OnDestroy(bInOwnerFinished);
}

void UAbilityTask_WaitForPushTargets_CapsuleTrace::PerformTrace()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UAbilityTask_WaitForPushTargets_CapsuleTrace);

	// Check if we have an avatar actor
	AActor* AvatarActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();
	if (!AvatarActor)
	{
		ActivateTimer(EPushPawnPauseType::ActivationFailed);
		return;
	}

	// Check if we have a world
	UWorld* World = GetWorld();
	if (!World)
	{
		ActivateTimer(EPushPawnPauseType::ActivationFailed);
		return;
	}

	// This can realistically occur before initialization passes
	if (!AvatarActor->HasActorBegunPlay())
	{
		ActivateTimer(EPushPawnPauseType::ActivationFailed);
		return;
	}

	// If we don't have a capsule component, we can't scan
	const UCapsuleComponent* CapsuleComponent = AvatarActor && AvatarActor->GetRootComponent() ?
		Cast<UCapsuleComponent>(AvatarActor->GetRootComponent()) : nullptr;
	
	if (!CapsuleComponent)
	{
		ActivateTimer(EPushPawnPauseType::ActivationFailed);
		return;
	}

	// Increase the capsule size based on the pushee's speed and acceleration
	bool bHasAcceleration = false;
	float VelocityScalar = 1.f;
	if (APawn* AvatarPawn = Cast<APawn>(AvatarActor))
	{
		// Check if the pushee is accelerating
		bHasAcceleration = UPushStatics::IsPusheeAccelerating(AvatarPawn);

		// Get the velocity scalar from the curve
		if (ScanParams.RadiusVelocityScalar)
		{
			VelocityScalar = ScanParams.RadiusVelocityScalar->GetFloatValue(UPushStatics::GetPawnGroundSpeed(AvatarPawn));
		}
	}

	// Calculate the radius scalar
	const float RadiusScalar = bHasAcceleration ? ScanParams.PusherRadiusAccelScalar : ScanParams.PusherRadiusScalar;

	// Create a capsule to trace with
	const float Radius = CapsuleComponent->GetScaledCapsuleRadius() * RadiusScalar * VelocityScalar;
	const float HalfHeight = CapsuleComponent->GetScaledCapsuleHalfHeight();
	FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(Radius, HalfHeight);

	// Initialize trace params
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(AvatarActor);
	
	constexpr bool bTraceComplex = false;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(UAbilityTask_WaitForPushTargets_CapsuleTrace), bTraceComplex);
	Params.AddIgnoredActors(ActorsToIgnore);

	// Perform the trace
	FVector TraceStart = StartLocation.GetTargetingTransform().GetLocation();
	FHitResult Hit;
	ShapeTrace(Hit, GetWorld(), TraceStart, ScanParams.TraceChannel, Params, CapsuleShape);

	// Append the push targets
	TArray<TScriptInterface<IPusherTarget>> PushTargets;
	UPushStatics::AppendPushTargetsFromHitResult(Hit, PushTargets);

	// Update the push options
	UpdatePushOptions(PushQuery, PushTargets);

#if ENABLE_DRAW_DEBUG
	if (FPushPawn::PushPawnScanDebugDraw)
	{
		FColor DebugColor = Hit.bBlockingHit ? FColor::Red : FColor::Green;
		DrawDebugCapsule(World, TraceStart, HalfHeight, Radius, FQuat::Identity, DebugColor, false, CurrentScanRate);
		if (Hit.bBlockingHit)
		{
			DrawDebugSphere(World, Hit.Location, 5, 16, DebugColor, false, CurrentScanRate);
		}
	}
#endif

	// Loop the timer
	ActivateTimer();
}

void UAbilityTask_WaitForPushTargets_CapsuleTrace::OnScanPaused(bool bIsPaused)
{
#if !UE_BUILD_SHIPPING
	if (FPushPawn::PushPawnPrintScanPaused > 0)
	{
		const bool bPrintServer = FPushPawn::PushPawnPrintScanPaused == 1 || FPushPawn::PushPawnPrintScanPaused == 2;
		const bool bPrintClient = FPushPawn::PushPawnPrintScanPaused == 1 || FPushPawn::PushPawnPrintScanPaused == 3;
		const bool bIsServer = GetAvatarActor()->HasAuthority();
		const bool bIsLocalClient = GetAvatarActor()->GetLocalRole() == ROLE_AutonomousProxy;
		if ((bPrintServer && bIsServer) || (bPrintClient && bIsLocalClient))
		{
			if (GEngine)
			{
				const FString NetRoleString = bIsServer ? TEXT("Server") : TEXT("Client");
				const FString PauseString = bIsPaused ? TEXT("Paused") : TEXT("Resumed");
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Emerald, FString::Printf(TEXT("PushPawn: %s scan %s"), *NetRoleString, *PauseString));
			}
		}
	}
#endif
	
	if (bIsPaused)
	{
		if (TimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		}
	}
	else
	{
		ActivateTimer();
	}
}
