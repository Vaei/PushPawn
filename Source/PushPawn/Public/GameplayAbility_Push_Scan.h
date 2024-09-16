// Copyright (c) Jared Taylor. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "PushOption.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayAbility_Push_Scan.generated.h"

/**
 * 
 */
UCLASS()
class PUSHPAWN_API UGameplayAbility_Push_Scan : public UGameplayAbility
{
	GENERATED_BODY()

public:
	/** If true, calculate the direction using X/Y only */
	UPROPERTY(EditDefaultsOnly, Category=Push)
	bool bDirectionIs2D;

	/** Channel to use when tracing for Pawns that can push us back */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Push)
	TEnumAsByte<ECollisionChannel> TraceChannel;

	UPROPERTY(BlueprintReadOnly, Category=Push)
	float ScanRange = 0.f;

	/** Modifies scan range */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Push)
	float RadiusScalar = 0.8f;

	/** Modifies scan range under acceleration */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Push)
	float RadiusAccelScalar = 1.1f;

	/** Scale radius by velocity */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Push)
	UCurveFloat* RadiusVelocityScalar = nullptr;
	
	/** How often to test for overlaps */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Push)
	float ScanRate = 0.1f;

	/** How often to test for overlaps under acceleration */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Push)
	float ScanRateAccel = 0.05f;
	
protected:
	UPROPERTY(BlueprintReadWrite, Category=Push)
	TArray<FPushOption> CurrentOptions;

public:
	UGameplayAbility_Push_Scan(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override final;

	UFUNCTION(BlueprintCallable, Category=Push)
	void UpdatePushs(const TArray<FPushOption>& PushOptions);

	UFUNCTION(BlueprintCallable, Category=Push)
	void TriggerPush();

	/**
	 * By default this is the largest size (halfheight vs radius) of our capsule, except the ScanRate could make it miss the initial points of contact so you may want to override this
	 * @see ActivateAbility - this does not update if changed after ActivateAbility
	 * @see GetMaxCapsuleSize - default return value
	 */
	UFUNCTION(BlueprintNativeEvent, Category=Push)
	float GetScanRange(const AActor* AvatarActor) const;

	/**
	 * Override to change scan rate based on runtime conditions
	 * @see ActivateAbility - this does not update if changed after ActivateAbility
	 */
	UFUNCTION(BlueprintNativeEvent, Category=Push)
	float GetScanRate(const AActor* AvatarActor) const;
	
	/**
	 * @return ScaledCapsuleHalfHeight or ScaledCapsuleRadius - whichever is larger - taken from class defaults (i.e ignores crouching character)
	 * @see ActivateAbility - this does not update if changed after ActivateAbility
	 */
	UFUNCTION(BlueprintPure, Category=Push)
	float GetMaxCapsuleSize(const AActor* AvatarActor) const;
};
