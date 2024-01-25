// Copyright (c) Jared Taylor. All Rights Reserved


#include "GameplayAbility_Push_Action.h"

UGameplayAbility_Push_Action::UGameplayAbility_Push_Action(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bServerRespectsRemoteAbilityCancellation = false;
	bRetriggerInstancedAbility = true;
}
