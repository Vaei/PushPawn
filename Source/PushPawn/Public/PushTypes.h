// Copyright (c) Jared Taylor. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "PushTypes.generated.h"

UENUM(BlueprintType)
enum class EPushCardinal : uint8
{
	Forward,
	Backward,
	Left,
	Right,
	ForwardLeft,
	ForwardRight,
	BackwardLeft,
	BackwardRight
};