// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PushPawn_Ability.h"
#include "PushPawn_Action_Base.generated.h"

/**
 * The base class for all PushPawn actions
 * This is a lightweight class that cannot use tags or other advanced features to reduce performance overhead
 * 
 * An action describes what happens when we walk into someone else, and they push us away
 * Note: We do not push them back, we are pushed away from them. It's an important distinction.
 *
 * The applied ability class should always be marked final. Implement in blueprint with apprehension - it costs a LOT of performance.
 */
UCLASS(Blueprintable)
class PUSHPAWN_API UPushPawn_Action_Base : public UPushPawn_Ability
{
	GENERATED_BODY()

public:
	virtual bool CanActivatePushPawnAbility(const AActor* AvatarActor) const override;
};
