// Copyright (c) Jared Taylor. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "PushTypes.h"
#include "GameplayAbility_Push_Action.generated.h"

UENUM(BlueprintType)
enum class EValidPushDirection : uint8
{
	ValidDirection,
	InvalidDirection
};

/**
 * You do not need this class, you only need to use the ctor defaults to get the correct functionality
 * GetPushDirection is static and can be used without this parent class
 */
UCLASS()
class PUSHPAWN_API UGameplayAbility_Push_Action : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_Push_Action(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Helper functions for extended functionality (not used in base plugin)
public:
	/** 
	 * Returns degree of the angle between Direction and Rotation forward vector
	 * The range of return will be from [-180, 180].
	 * @param	Direction		The direction relative to BaseRotation
	 * @param	BaseRotation	The base rotation, e.g. of a pawn
	 */
	static float CalculateDirection(const FVector& Direction, const FRotator& BaseRotation);

	UFUNCTION(BlueprintCallable, Category=Push, meta=(ExpandEnumAsExecs="ValidPushDirection"))
	static EPushCardinal GetPushDirection(const AActor* FromActor, const AActor* ToActor, EValidPushDirection& ValidPushDirection);
};
