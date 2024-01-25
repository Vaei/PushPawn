// Copyright (c) Jared Taylor. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayAbility_Push_Action.generated.h"

/**
 * You do not need this class, you only need to use the ctor defaults to get the correct functionality
 */
UCLASS()
class PUSHPAWN_API UGameplayAbility_Push_Action : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_Push_Action(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
