// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PushQuery.generated.h"


/**  */
USTRUCT(BlueprintType)
struct FPushQuery
{
	GENERATED_BODY()

public:
	/** The requesting pawn who will be getting pushed */
	UPROPERTY(BlueprintReadWrite, Category=Push)
	TWeakObjectPtr<AActor> RequestingAvatar;

	/** A generic UObject to shove in extra data required for the Push */
	UPROPERTY(BlueprintReadWrite, Category=Push)
	TWeakObjectPtr<UObject> OptionalObjectData;
};
