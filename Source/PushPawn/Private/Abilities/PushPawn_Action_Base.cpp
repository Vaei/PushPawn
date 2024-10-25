// Copyright (c) Jared Taylor. All Rights Reserved


#include "Abilities/PushPawn_Action_Base.h"

#include "IPush.h"
#include "PushStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PushPawn_Action_Base)

bool UPushPawn_Action_Base::CanActivatePushPawnAbility(const AActor* AvatarActor) const
{
	if (!AvatarActor)
	{
		return false;
	}
	
	if (const IPusheeInstigator* Interface = UPushStatics::GetPusheeInstigator(AvatarActor))
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

bool UPushPawn_Action_Base::WantsPushPawnActionDebugDraw()
{
#if ENABLE_DRAW_DEBUG
	static const IConsoleVariable* CVarPushPawnActionDebugDraw = IConsoleManager::Get().FindConsoleVariable(TEXT("p.PushPawn.Action.Debug.Draw"));
	const int32 PushPawnActionDebugDraw = CVarPushPawnActionDebugDraw ? CVarPushPawnActionDebugDraw->GetInt() : 0;
	return PushPawnActionDebugDraw > 0;
#else
	return false;
#endif
}
