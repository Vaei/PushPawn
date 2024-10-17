// Copyright (c) Jared Taylor. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "PushOption.h"
#include "IPush.generated.h"

class IPusherTarget;
struct FPushQuery;

/**  */
class FPushOptionBuilder
{
public:
	FPushOptionBuilder(TScriptInterface<IPusherTarget> InterfaceTargetScope, TArray<FPushOption>& InteractOptions)
		: Scope(InterfaceTargetScope)
		, Options(InteractOptions)
	{
	}

	void AddPushOption(const FPushOption& Option) const
	{
		FPushOption& OptionEntry = Options.Add_GetRef(Option);
		OptionEntry.PusherTarget = Scope;
	}

private:
	TScriptInterface<IPusherTarget> Scope;
	TArray<FPushOption>& Options;
};

/** Interface for the instigator who gets pushed */
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UPusheeInstigator : public UInterface
{
	GENERATED_BODY()
};

/** Interface for the instigator who gets pushed */
class IPusheeInstigator
{
	GENERATED_BODY()

public:
	virtual bool CanBePushed(const AActor* PusherActor) const = 0;
};

/** Interface for the target who does the pushing */
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UPusherTarget : public UInterface
{
	GENERATED_BODY()
};

/**  */
class IPusherTarget
{
	GENERATED_BODY()

public:
	/**  */
	virtual void GatherPushOptions(const FPushQuery& PushQuery, FPushOptionBuilder& OptionBuilder) = 0;

	/**  */
	virtual void CustomizePushEventData(const FGameplayTag& PushEventTag, FGameplayEventData& InOutEventData) { }

	virtual bool CanPush(const AActor* PusheeActor) const = 0;
};