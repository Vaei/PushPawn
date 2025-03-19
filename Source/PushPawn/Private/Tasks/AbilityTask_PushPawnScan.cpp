// Copyright (c) Jared Taylor. All Rights Reserved

#include "Tasks/AbilityTask_PushPawnScan.h"

#include "GameFramework/Pawn.h"
#include "AbilitySystemComponent.h"
#include "Abilities/PushPawn_Scan_Base.h"
#include "Tasks/AbilityTask_PushPawnSync.h"
#include "IPush.h"
#include "PushStatics.h"
#include "PushQuery.h"

#include "TimerManager.h"
#include "Curves/CurveFloat.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "CollisionShape.h"

#if !UE_BUILD_SHIPPING
#include "Engine/Engine.h"
#include "Logging/MessageLog.h"
#endif

#if UE_BUILD_SHIPPING
#include "AbilitySystemLog.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(AbilityTask_PushPawnScan)

namespace FPushPawnCVars
{
#if UE_ENABLE_DEBUG_DRAWING
	static int32 PushPawnScanDebugDraw = 0;
	FAutoConsoleVariableRef CVarPushPawnScanDebugDraw(
		TEXT("p.PushPawn.Scan.Debug.Draw"),
		PushPawnScanDebugDraw,
		TEXT("Optionally draw debug for PushPawn Scan.\n")
		TEXT("0: Disable, 1: Enable"),
		ECVF_Default);
#endif

#if !UE_BUILD_SHIPPING
	static bool bPushPawnDisabled = false;
	FAutoConsoleVariableRef CVarPushPawnDisabled(
		TEXT("p.PushPawn.Disable"),
		bPushPawnDisabled,
		TEXT("Disable PushPawn. Will continue running tasks, but Pushes will never occur.\n"),
		ECVF_Cheat);
	
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

UAbilityTask_PushPawnScan* UAbilityTask_PushPawnScan::PushPawnScan(
	UGameplayAbility* OwningAbility,
	FPushQuery PushQuery,
	FGameplayAbilityTargetingLocationInfo StartLocation, const FPushPawnScanParams& ScanParams, float ActivationFailureDelay
)
{
	UAbilityTask_PushPawnScan* MyObj = NewAbilityTask<UAbilityTask_PushPawnScan>(OwningAbility);
	MyObj->ScanParams = ScanParams;
	MyObj->StartLocation = StartLocation;
	MyObj->PushQuery = PushQuery;
	MyObj->ActivationFailureDelay = ActivationFailureDelay;

	return MyObj;
}

void UAbilityTask_PushPawnScan::OnNetSync(UAbilityTask_PushPawnSync* SyncPoint)
{
	// Remove finished net sync
	if (IsValid(SyncPoint))
	{
		SyncPoints.RemoveSingle(SyncPoint);
	}
	
	// Re-activate the timer
	ActivateTimer();
}

void UAbilityTask_PushPawnScan::ActivateTimer(EPushPawnPauseType PauseType)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UAbilityTask_PushPawnScan::ActivateTimer);

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
		if (FPushPawnCVars::PushPawnPrintNetSync > 0)
		{
			const bool bPrintServer = FPushPawnCVars::PushPawnPrintNetSync == 1 || FPushPawnCVars::PushPawnPrintNetSync == 2;
			const bool bPrintClient = FPushPawnCVars::PushPawnPrintNetSync == 1 || FPushPawnCVars::PushPawnPrintNetSync == 3;
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
		
		UAbilityTask_PushPawnSync* WaitNetSync = UAbilityTask_PushPawnSync::WaitNetSync(Ability);
		WaitNetSync->OnSync.AddDynamic(this, &ThisClass::OnNetSync);
		WaitNetSync->ReadyForActivation();
		SyncPoints.Add(WaitNetSync);
		return;
	}

	if (!OnPushPawnScanPauseStateChangedDelegate)
	{
		if (IPusheeInstigator* Pushee = UPushStatics::GetPusheeInstigator(GetAvatarActor()))
		{
			OnPushPawnScanPauseStateChangedDelegate = Pushee->GetPushPawnScanPausedDelegate();
			if (OnPushPawnScanPauseStateChangedDelegate)
			{
				OnPushPawnScanPauseStateChangedDelegate->BindUObject(this, &ThisClass::OnScanPaused);
			}
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
		Delegate.BindWeakLambda(this, PendingTimer);

		// Set the timer
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, Delegate, ActivationFailureDelay, false);
		}
	}
}

void UAbilityTask_PushPawnScan::Activate()
{
	SetWaitingOnAvatar();

	ActivateTimer();
}

void UAbilityTask_PushPawnScan::OnDestroy(bool bInOwnerFinished)
{
	// Clear the timer
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

		// Clear non-uobject timers
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	}

	// Unbind the delegate
	if (OnPushPawnScanPauseStateChangedDelegate)
	{
		OnPushPawnScanPauseStateChangedDelegate->Unbind();
		OnPushPawnScanPauseStateChangedDelegate = nullptr;
	}

	for (UAbilityTask_PushPawnSync* WaitNetSync : SyncPoints)
	{
		if (IsValid(WaitNetSync))
		{
			WaitNetSync->EndTask();
		}
	}

	Super::OnDestroy(bInOwnerFinished);
}

void UAbilityTask_PushPawnScan::PerformTrace()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UAbilityTask_PushPawnScan::PerformTrace);

#if !UE_BUILD_SHIPPING
	if (FPushPawnCVars::bPushPawnDisabled)
	{
		OnDisabledDelegate.BindLambda([this](const IConsoleVariable* CVar)
		{
			const bool bDisabled = CVar->GetBool();
			if (!bDisabled)
			{
				FPushPawnCVars::CVarPushPawnDisabled->SetOnChangedCallback(nullptr);
				ActivateTimer();
			}
			else
			{
				if (OnDisabledDelegate.IsBound())
				{
					OnDisabledDelegate.Unbind();
				}
			}
		});
		FPushPawnCVars::CVarPushPawnDisabled->SetOnChangedCallback(OnDisabledDelegate);
		return;
	}
#endif
	
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

	const IPusheeInstigator* Pushee = UPushStatics::GetPusheeInstigator(AvatarActor);
	if (!Pushee)
    {
		const FString ErrorString = FString::Printf(TEXT("PushPawn: Avatar actor %s does not implement IPusheeInstigator!"), *AvatarActor->GetName());
#if !UE_BUILD_SHIPPING
		if (IsInGameThread())
		{
			FMessageLog("PIE").Error(FText::FromString(ErrorString));
		}
#else
		ABILITY_LOG(Error, TEXT("%s"), *ErrorString);
#endif
        return;
    }

	// If we don't have a valid collision shape, we can't scan
	FQuat ShapeRotation;
	FCollisionShape CollisionShape = Pushee->GetPusheeCollisionShape(ShapeRotation);
	const bool bValidShape = !CollisionShape.IsLine() && !CollisionShape.IsNearlyZero();
	if (!ensureMsgf(bValidShape, TEXT("PushPawn: Pushee %s has an invalid collision shape!"), *AvatarActor->GetName()))
    {
        ActivateTimer(EPushPawnPauseType::ActivationFailed);
        return;
    }

	// Increase the collision size based on the pushee's speed and acceleration
	float VelocityScalar = 1.f;
	
	// Check if the pushee is accelerating
	const bool bHasAcceleration = UPushStatics::IsPusheeAccelerating(Pushee);

	// Get the velocity scalar from the curve
	if (ScanParams.RadiusVelocityScalar)
	{
		VelocityScalar = ScanParams.RadiusVelocityScalar->GetFloatValue(UPushStatics::GetPusheeGroundSpeed(Pushee));
	}

	// Calculate the radius scalar
	const float RadiusScalar = bHasAcceleration ? ScanParams.PusheeRadiusAccelScalar : ScanParams.PusheeRadiusScalar;

	// Create a collision shape to trace with
	const float ShapeScalar = RadiusScalar * VelocityScalar;
	switch (CollisionShape.ShapeType)
	{
		case ECollisionShape::Box:
		{
			CollisionShape.Box.HalfExtentX *= ShapeScalar;
			CollisionShape.Box.HalfExtentY *= ShapeScalar;
		}
		break;
		case ECollisionShape::Sphere:
		{
			CollisionShape.Sphere.Radius *= ShapeScalar;
		}
		break;
		case ECollisionShape::Capsule:
		{
			CollisionShape.Capsule.Radius *= ShapeScalar;
		}
		break;
		default: break;
	}

	// Initialize trace params
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(AvatarActor);
	
	constexpr bool bTraceComplex = false;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(UAbilityTask_PushPawnScan_Trace), bTraceComplex);
	Params.AddIgnoredActors(ActorsToIgnore);

	// Perform the trace
	const FVector TraceStart = StartLocation.GetTargetingTransform().GetLocation();
	FHitResult Hit;
	ShapeTrace(Hit, GetWorld(), TraceStart, ShapeRotation, ScanParams.TraceChannel, Params, CollisionShape);

	// Append the push targets
	TArray<TScriptInterface<IPusherTarget>> PushTargets;
	UPushStatics::AppendPushTargetsFromHitResult(Hit, PushTargets);

	// Update the push options
	UpdatePushOptions(PushQuery, PushTargets);

#if UE_ENABLE_DEBUG_DRAWING
	if (FPushPawnCVars::PushPawnScanDebugDraw)
	{
		FColor DebugColor = Hit.bBlockingHit ? FColor::Red : FColor::Green;
		
		switch (CollisionShape.ShapeType)
		{
			case ECollisionShape::Box:
			{
				DrawDebugBox(World, TraceStart, CollisionShape.GetExtent(), ShapeRotation, DebugColor, false, CurrentScanRate);
			}
			break;
			case ECollisionShape::Sphere:
			{
				DrawDebugSphere(World, TraceStart, CollisionShape.GetCapsuleRadius(), 16, DebugColor, false, CurrentScanRate);
			}
			break;
			case ECollisionShape::Capsule:
			{
				DrawDebugCapsule(World, TraceStart, CollisionShape.GetCapsuleHalfHeight(), CollisionShape.GetCapsuleRadius(), ShapeRotation, DebugColor, false, CurrentScanRate);
			}
			break;
			default: break;
		}
		
		if (Hit.bBlockingHit)
		{
			DrawDebugSphere(World, Hit.Location, 5, 16, DebugColor, false, CurrentScanRate);
		}
	}
#endif

	// Loop the timer
	ActivateTimer();
}

void UAbilityTask_PushPawnScan::OnScanPaused(bool bIsPaused)
{
#if !UE_BUILD_SHIPPING
	if (FPushPawnCVars::PushPawnPrintScanPaused > 0)
	{
		const bool bPrintServer = FPushPawnCVars::PushPawnPrintScanPaused == 1 || FPushPawnCVars::PushPawnPrintScanPaused == 2;
		const bool bPrintClient = FPushPawnCVars::PushPawnPrintScanPaused == 1 || FPushPawnCVars::PushPawnPrintScanPaused == 3;
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
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
		Pause();
	}
	else
	{
		Resume();
		ActivateTimer();
	}
}
