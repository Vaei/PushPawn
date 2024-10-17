// Copyright (c) Jared Taylor. All Rights Reserved

#include "Tasks/AbilityTask_WaitForPushTargets_CapsuleTrace.h"
#include "GameFramework/Actor.h"
#include "IPush.h"
#include "PushStatics.h"
#include "PushQuery.h"
#include "AbilitySystemComponent.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

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
		ECVF_Cheat);
#endif
}

UAbilityTask_WaitForPushTargets_CapsuleTrace::UAbilityTask_WaitForPushTargets_CapsuleTrace(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UAbilityTask_WaitForPushTargets_CapsuleTrace* UAbilityTask_WaitForPushTargets_CapsuleTrace::WaitForPushTargets_CapsuleTrace(
	UGameplayAbility* OwningAbility,
	FPushQuery PushQuery,
	ECollisionChannel TraceChannel,
	FGameplayAbilityTargetingLocationInfo StartLocation,
	float RadiusScalar,
	float RadiusAccelScalar,
	UCurveFloat* VelocityRadiusScalar,
	float PushScanRate,
	float PushScanRateAccel
	)
{
	UAbilityTask_WaitForPushTargets_CapsuleTrace* MyObj = NewAbilityTask<UAbilityTask_WaitForPushTargets_CapsuleTrace>(OwningAbility);
	MyObj->RadiusScalar = RadiusScalar;
	MyObj->RadiusAccelScalar = RadiusAccelScalar;
	MyObj->VelocityRadiusScalar = VelocityRadiusScalar;
	MyObj->PushScanRate = PushScanRate;
	MyObj->PushScanRateAccel = PushScanRateAccel;
	MyObj->StartLocation = StartLocation;
	MyObj->PushQuery = PushQuery;
	MyObj->TraceChannel = TraceChannel;

	return MyObj;
}

void UAbilityTask_WaitForPushTargets_CapsuleTrace::ActivateTimer(bool bHasAccel)
{
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ThisClass::PerformTrace, (bHasAccel ? PushScanRateAccel : PushScanRate), true);
}

void UAbilityTask_WaitForPushTargets_CapsuleTrace::Activate()
{
	SetWaitingOnAvatar();

	ActivateTimer();
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
		ActivateTimer();
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		ActivateTimer();
		return;
	}

	// This can realistically occur before initialization passes
	if (!AvatarActor->HasActorBegunPlay())
	{
		ActivateTimer();
		return;
	}

	// Search around us
	const UCapsuleComponent* CapsuleComponent = AvatarActor && AvatarActor->GetRootComponent() ?
		Cast<UCapsuleComponent>(AvatarActor->GetRootComponent()) : nullptr;

	if (!CapsuleComponent)
	{
		ActivateTimer();
		return;
	}

	bool bHasAcceleration = false;
	float VelocityScalar = 1.f;
	if (ACharacter* Character = Cast<ACharacter>(AvatarActor))
	{
		if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
		{
			bHasAcceleration = !Movement->GetCurrentAcceleration().IsNearlyZero(10.f);
		}

		if (VelocityRadiusScalar)
		{
			const float VelocityMag = Character->GetVelocity().Size2D();
			VelocityScalar = VelocityRadiusScalar->GetFloatValue(VelocityMag);
		}
	}

	ActivateTimer(bHasAcceleration);

	const float Radius = CapsuleComponent->GetScaledCapsuleRadius() * (bHasAcceleration ? RadiusAccelScalar : RadiusScalar) * VelocityScalar;
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
	if (FPushPawn::PushPawnScanDebugDraw)
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
