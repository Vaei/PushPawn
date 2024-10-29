// Copyright (c) Jared Taylor. All Rights Reserved


#include "Abilities/PushPawn_Ability.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemLog.h"

#if WITH_EDITOR
#include "Logging/MessageLog.h"
#include "Misc/UObjectToken.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(PushPawn_Ability)

#define LOCTEXT_NAMESPACE "PushPawnAbility"

UPushPawn_Ability::UPushPawn_Ability(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	bServerRespectsRemoteAbilityCancellation = false;
	bRetriggerInstancedAbility = true;
}

bool UPushPawn_Ability::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UPushPawn_Ability::CanActivateAbility);
	
	// A valid AvatarActor is required.
	AActor* const AvatarActor = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	if (AvatarActor == nullptr)
	{
		return false;
	}

	// Simulated proxy check means only authority or autonomous proxies should be executing abilities.
	if (!ShouldActivateAbility(AvatarActor->GetLocalRole()))
	{
		return false;
	}

	// Make sure the ability system component is valid, if not bail out.
	UAbilitySystemComponent* const AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get();
	if (!AbilitySystemComponent)
	{
		return false;
	}
	
	FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
	if (!Spec)
	{
		ABILITY_LOG(Warning, TEXT("CanActivateAbility %s failed, called with invalid Handle"), *GetName());
		return false;
	}

	if (!bAllowBlueprintImplementation)
	{
		ensureAlwaysMsgf(!bHasBlueprintCanUse, TEXT("Blueprint logic is not allowed in this ability { %s }."), *GetName());
	}

	return CanActivatePushPawnAbility(AvatarActor);
}

void UPushPawn_Ability::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	ActivatePushPawnAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (bAllowBlueprintImplementation)
	{
		Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	}
	else
	{
#if WITH_EDITOR
		if (bHasBlueprintActivate || bHasBlueprintActivateFromEvent)
		{
			FMessageLog("PIE").Error()
				->AddToken(FUObjectToken::Create(this->GetClass()))
				->AddToken(FUObjectToken::Create(ActorInfo->AvatarActor.Get()->GetClass()))
				->AddToken(FTextToken::Create(LOCTEXT("ActivatePushAbilityError1", " UPushPawn_Ability::ActivateAbility has Blueprint implementation but Blueprint logic is not allowed because bAllowBlueprintImplementation is false."))
			);
		}
#endif
		CommitAbility(Handle, ActorInfo, ActivationInfo);
	}
}

#undef LOCTEXT_NAMESPACE