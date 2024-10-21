// Copyright (c) Jared Taylor. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "PushPawn_Ability.generated.h"

/**
 * The minimal ability class used for PushPawn
 * This is a lightweight class that cannot use tags or other advanced features to reduce performance overhead
 * 
 * This scans for nearby pawns who push us away from them
 * Note: We do not push them back, we are pushed away from them. It's an important distinction.
 */
UCLASS(NotBlueprintable)
class PUSHPAWN_API UPushPawn_Ability : public UGameplayAbility
{
	GENERATED_BODY()

protected:
	/** Blueprint logic runs in a virtual machine with severe performance overhead. This ability is activated very often. */
	UPROPERTY(EditDefaultsOnly, AdvancedDisplay, Category=PushPawn)
	bool bAllowBlueprintImplementation = false;
	
public:
	UPushPawn_Ability(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual bool CanActivatePushPawnAbility(const AActor* AvatarActor) const { return true; }

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
		FGameplayTagContainer* OptionalRelevantTags) const override final;

protected:
	/** @return True if the ability was not ended or cancelled */
	virtual bool ActivatePushPawnAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) { return true; }
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override final;
};
