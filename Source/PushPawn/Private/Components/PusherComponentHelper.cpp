// Copyright (c) Jared Taylor. All Rights Reserved.


#include "Components/PusherComponentHelper.h"

#include "PushStatics.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(PusherComponentHelper)

void UPusherComponentHelper::GatherPushOptions(const FPushQuery& PushQuery, FPushOptionBuilder& OptionBuilder)
{
	UPushStatics::GatherPushOptions(K2_GetPushAbility(), PawnOwner, PushQuery, OptionBuilder);
}
