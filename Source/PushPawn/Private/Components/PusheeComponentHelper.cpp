// Copyright (c) Jared Taylor. All Rights Reserved


#include "Components/PusheeComponentHelper.h"

#include "PushStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PusheeComponentHelper)

FPushPawnCollisionShapeHelper UPusheeComponentHelper::K2_GetDefaultPusheeCollisionShape(const AActor* Actor,
	FQuat& ShapeRotation, EPushCollisionType OptionalShapeType, USceneComponent* OptionalComponent) const
{
	if (!IsValid(Actor))
	{
		return {};
	}
	const FCollisionShape Shape = UPushStatics::GetDefaultPusheeCollisionShape(Actor, ShapeRotation, OptionalShapeType, OptionalComponent);
	return FPushPawnCollisionShapeHelper { Shape };
}
