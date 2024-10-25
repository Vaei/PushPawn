// Copyright (c) Jared Taylor. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "PushTypes.generated.h"

UENUM(BlueprintType)
enum class EPushCollisionType : uint8
{
	None,
	Capsule,
	Box,
	Sphere,
};

/**
 * The direction the pawn is pushed in, used for selecting a directional montage
 */
UENUM(BlueprintType)
enum class EPushCardinal : uint8
{
	Forward,
	Backward,
	Left,
	Right,
	ForwardLeft,
	ForwardRight,
	BackwardLeft,
	BackwardRight
};

UENUM(BlueprintType)
enum class EValidPushDirection : uint8
{
	ValidDirection,
	InvalidDirection
};

UENUM(BlueprintType)
enum class EPushPawnPauseType : uint8
{
	NotPaused,
	ActivationFailed,
};

/**
 * Customize the behavior of the pawn when it is pushed
 */
USTRUCT(BlueprintType)
struct PUSHPAWN_API FPushPawnActionParams
{
	GENERATED_BODY()

	FPushPawnActionParams()
		: VelocityToStrengthCurve(nullptr)
		, Duration(0.1f)
		, StrengthScalar(1.f)
	{}

	/** Scale the push strength by the pushee's own velocity */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=PushPawn)
	UCurveFloat* VelocityToStrengthCurve;

	/** How long the push force lasts for */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=PushPawn)
	float Duration;

	/** Scales VelocityToStrengthCurve if available, otherwise directly sets the Strength of the push */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=PushPawn)
	float StrengthScalar;
};

/**
 * Adjust the behavior of the pawn when scanning for other pawns
 */
USTRUCT(BlueprintType)
struct PUSHPAWN_API FPushPawnScanParams
{
	GENERATED_BODY()

	FPushPawnScanParams()
		: bDirectionIs2D(true)
		, TraceChannel(ECC_Visibility)
		, ScanRangeScalar(0.8f)
		, ScanRangeAccelScalar(1.1f)
		, PusheeRadiusScalar(0.8f)
		, PusheeRadiusAccelScalar(1.0f)
		, RadiusVelocityScalar(nullptr)
		, ScanRate(0.1f)
		, ScanRateAccel(0.05f)
	{}

	/** If true, calculate the direction using X/Y only */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=PushPawn)
	bool bDirectionIs2D;

	/** Channel to use when tracing for Pawns that can push us back */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=PushPawn)
	TEnumAsByte<ECollisionChannel> TraceChannel;

	/** Modifies scan range */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=PushPawn)
	float ScanRangeScalar;

	/** Modifies scan range under acceleration */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=PushPawn)
	float ScanRangeAccelScalar;

	/** Change the size we consider the pusher (the one who pushes us) to be when searching for them */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=PushPawn)
	float PusheeRadiusScalar;

	/** Change the size we consider the pusher (the one who pushes us) to be when searching for them when WE are under acceleration */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=PushPawn)
	float PusheeRadiusAccelScalar;

	/** Change the size we consider the pusher (the one who pushes us) to be when searching for them based on OUR velocity */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=PushPawn)
	UCurveFloat* RadiusVelocityScalar;
	
	/** How often to test for overlaps */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=PushPawn)
	float ScanRate;

	/** How often to test for overlaps under acceleration */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=PushPawn)
	float ScanRateAccel;
};

/**
 * Blueprint cannot use FCollisionShape, so we need to create a helper struct
 */
USTRUCT(BlueprintType)
struct PUSHPAWN_API FPushPawnCollisionShapeHelper
{
	GENERATED_BODY()

	FPushPawnCollisionShapeHelper()
		: CollisionType(EPushCollisionType::Capsule)
		, BoxHalfExtent(FVector(34.f, 34.f, 88.f))
		, SphereRadius(34.f)
	{}

	FPushPawnCollisionShapeHelper(const FCollisionShape& Shape)
	{
		FromCollisionShape(Shape);
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=PushPawn)
	EPushCollisionType CollisionType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=PushPawn, meta=(EditCondition="CollisionType == EPushCollisionType::Capsule"))
	float CapsuleRadius = 34.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=PushPawn, meta=(EditCondition="CollisionType == EPushCollisionType::Capsule"))
	float CapsuleHalfHeight = 88.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=PushPawn, meta=(EditCondition="CollisionType == EPushCollisionType::Box"))
	FVector BoxHalfExtent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=PushPawn, meta=(EditCondition="CollisionType == EPushCollisionType::Sphere"))
	float SphereRadius;

	FCollisionShape ToCollisionShape() const;
	void FromCollisionShape(const FCollisionShape& Shape);
};
