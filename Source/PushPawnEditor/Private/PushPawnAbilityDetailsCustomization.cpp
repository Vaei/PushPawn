// Copyright (c) Jared Taylor. All Rights Reserved


#include "PushPawnAbilityDetailsCustomization.h"

#include "DetailLayoutBuilder.h"
#include "Abilities/PushPawn_Ability.h"


TSharedRef<IDetailCustomization> FPushPawnAbilityDetailsCustomization::MakeInstance()
{
	return MakeShared<FPushPawnAbilityDetailsCustomization>();
}

void FPushPawnAbilityDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);

	for (const TWeakObjectPtr<UObject>& Object : Objects)
	{
		if (UPushPawn_Ability* PushPawnAbility = Cast<UPushPawn_Ability>(Object.Get()))
		{
			// Bump PushPawn to the top of the details panel
			DetailBuilder.EditCategory(TEXT("PushPawn"), FText::GetEmpty(), ECategoryPriority::Transform);

			// We use a lightweight CanActivateAbility(), so we can remove unavailable tags
			DetailBuilder.HideProperty("ActivationRequiredTags", UGameplayAbility::StaticClass());
			DetailBuilder.HideProperty("ActivationBlockedTags", UGameplayAbility::StaticClass());
			DetailBuilder.HideProperty("SourceRequiredTags", UGameplayAbility::StaticClass());
			DetailBuilder.HideProperty("SourceBlockedTags", UGameplayAbility::StaticClass());
			DetailBuilder.HideProperty("TargetRequiredTags", UGameplayAbility::StaticClass());
			DetailBuilder.HideProperty("TargetBlockedTags", UGameplayAbility::StaticClass());

			// Unavailable properties
			DetailBuilder.HideProperty("CostGameplayEffectClass", UGameplayAbility::StaticClass());
			DetailBuilder.HideProperty("CooldownGameplayEffectClass", UGameplayAbility::StaticClass());

			// Irrelevant properties (handled internally)
			DetailBuilder.HideProperty("AbilityTriggers", UGameplayAbility::StaticClass());

			// Avoid designers causing astronomical bandwidth costs
			DetailBuilder.HideProperty("bReplicateInputDirectly", UGameplayAbility::StaticClass());
			DetailBuilder.HideProperty("ReplicationPolicy", UGameplayAbility::StaticClass());
		}
	}
}