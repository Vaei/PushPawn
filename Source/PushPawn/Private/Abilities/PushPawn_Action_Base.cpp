// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/PushPawn_Action_Base.h"

#include "IPush.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PushPawn_Action_Base)

bool UPushPawn_Action_Base::CanActivatePushPawnAbility(const AActor* AvatarActor) const
{
	if (!AvatarActor)
	{
		return false;
	}
	
	if (const IPusheeInstigator* Interface = Cast<IPusheeInstigator>(AvatarActor))
	{
		if (Interface->IsPushable())
		{
			return true;
		}
	}
	else
	{
		return false;
	}

	return false;
}
