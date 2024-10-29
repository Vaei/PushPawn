// Copyright (c) Jared Taylor. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PushPawnComponent.generated.h"

class ACharacter;

/**
 * Base component for PushPawn
 * Includes caching helper UpdatePawnOwner() and K2_UpdatePawnOwner()
 */
UCLASS(Abstract)
class PUSHPAWN_API UPushPawnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Owning Pawn if Owner is a Pawn */
	UPROPERTY(Transient, DuplicateTransient, BlueprintReadOnly, Category=PushPawn)
	TObjectPtr<APawn> PawnOwner;

	/** Owning Character if Owner is a Character */
	UPROPERTY(Transient, DuplicateTransient, BlueprintReadOnly, Category=PushPawn)
	TObjectPtr<ACharacter> CharacterOwner;

public:
	UPushPawnComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void OnRegister() override;
	virtual void InitializeComponent() override;
	virtual void PostLoad() override;

	void UpdatePawnOwner();

	/**
	 * Cache your blueprint-derived owning pawn or character here
	 * APawn and ACharacter are already cached for you
	 * @see PawnOwner, @see CharacterOwner
	 */
	UFUNCTION(BlueprintImplementableEvent, Category=PushPawn, meta=(DisplayName="Update Pawn Owner"))
	void K2_UpdatePawnOwner();
};
