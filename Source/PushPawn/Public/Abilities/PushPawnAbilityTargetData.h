// Copyright (c) Jared Taylor. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "PushPawnAbilityTargetData.generated.h"


/**
 * Ability target data that holds a direction vector, used for push abilities to send the direction of the push
 */
USTRUCT(BlueprintType)
struct FPushPawnAbilityTargetData : public FGameplayAbilityTargetData
{
	GENERATED_BODY()

	FPushPawnAbilityTargetData()
		: Direction(FVector::ZeroVector)
	{}

	FPushPawnAbilityTargetData(const FVector& InDirection)
		: Direction(InDirection)
	{}

	UPROPERTY(BlueprintReadOnly, Category=Character)
	FVector_NetQuantizeNormal Direction;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		Direction.NetSerialize(Ar, Map, bOutSuccess);
		return true;
	}
	
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return StaticStruct();
	}
};

template<>
struct TStructOpsTypeTraits<FPushPawnAbilityTargetData> : public TStructOpsTypeTraitsBase2<FPushPawnAbilityTargetData>
{
	enum
	{
		WithNetSerializer = true	// For now this is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
	};
};