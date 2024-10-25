// Copyright (c) Jared Taylor. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "PushQuery.generated.h"


/**
 * Information required to push a pawn.
 */
USTRUCT(BlueprintType)
struct PUSHPAWN_API FPushQuery
{
	GENERATED_BODY()

public:
	/** The requesting pawn who will be getting pushed */
	UPROPERTY(BlueprintReadWrite, Category=PushPawn)
	TWeakObjectPtr<AActor> RequestingAvatar;

	/** A generic UObject to shove in extra data required for the Push */
	UPROPERTY(BlueprintReadWrite, Category=PushPawn)
	TWeakObjectPtr<UObject> OptionalObjectData;
};
