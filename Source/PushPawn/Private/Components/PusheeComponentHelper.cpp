// Copyright (c) Jared Taylor. All Rights Reserved


#include "Components/PusheeComponentHelper.h"

#include "PushStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PusheeComponentHelper)

FCollisionShape FPushPawnCollisionShapeHelper::ToCollisionShape() const
{
	switch (CollisionType)
	{
	case EPushCollisionType::Capsule: return FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);
	case EPushCollisionType::Box: return FCollisionShape::MakeBox(BoxHalfExtent);
	case EPushCollisionType::Sphere: return FCollisionShape::MakeSphere(SphereRadius);
	default: return {};
	}
}

void FPushPawnCollisionShapeHelper::FromCollisionShape(const FCollisionShape& Shape)
{
	switch (Shape.ShapeType)
	{
	case ECollisionShape::Capsule:
		CollisionType = EPushCollisionType::Capsule;
		CapsuleRadius = Shape.Capsule.Radius;
		CapsuleHalfHeight = Shape.Capsule.HalfHeight;
		break;
	case ECollisionShape::Box:
		CollisionType = EPushCollisionType::Box;
		BoxHalfExtent = Shape.GetExtent();
		break;
	case ECollisionShape::Sphere:
		CollisionType = EPushCollisionType::Sphere;
		SphereRadius = Shape.Sphere.Radius;
		break;
	default:
		CollisionType = EPushCollisionType::None;
		break;
	}
}

FPushPawnCollisionShapeHelper UPusheeComponentHelper::K2_GetDefaultPusheeCollisionShape(const AActor* Actor,
	EPushCollisionType OptionalShapeType, USceneComponent* OptionalComponent) const
{
	if (!IsValid(Actor))
	{
		return {};
	}
	FCollisionShape Shape = UPushStatics::GetDefaultPusheeCollisionShape(Actor, OptionalShapeType, OptionalComponent);
	return FPushPawnCollisionShapeHelper { Shape };
}
