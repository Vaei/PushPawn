// Copyright (c) Jared Taylor. All Rights Reserved


#include "GameplayAbility_Push_Action.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayAbility_Push_Action)

UGameplayAbility_Push_Action::UGameplayAbility_Push_Action(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bServerRespectsRemoteAbilityCancellation = false;
	bRetriggerInstancedAbility = true;
}
