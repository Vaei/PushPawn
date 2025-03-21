// Copyright (c) Jared Taylor. All Rights Reserved


#include "PushTypes.h"

#include "Engine/EngineTypes.h"
#include "CollisionShape.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PushTypes)

FPushPawnScanParams::FPushPawnScanParams()
	: bDirectionIs2D(true)
	, TraceChannel(ECC_Visibility)
    , ScanRangeScalar(0.8f)
    , ScanRangeAccelScalar(1.1f)
    , PusheeRadiusScalar(0.8f)
    , PusheeRadiusAccelScalar(1.0f)
    , RadiusVelocityScalar(nullptr)
    , ScanRate(0.1f)
    , ScanRateAccel(0.05f)
	, StrengthOverrideHandling(EPushPawnOverrideHandling::Min)
{}

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
