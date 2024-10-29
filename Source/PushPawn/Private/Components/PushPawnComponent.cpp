// Copyright (c) Jared Taylor. All Rights Reserved


#include "Components/PushPawnComponent.h"

#include "GameFramework/Character.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PushPawnComponent)

UPushPawnComponent::UPushPawnComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UPushPawnComponent::OnRegister()
{
	Super::OnRegister();

	if (GetWorld() && GetWorld()->IsGameWorld())
	{
		UpdatePawnOwner();
	}
	
	K2_UpdatePawnOwner();  // PostLoad() will crash
}

void UPushPawnComponent::InitializeComponent()
{
	Super::InitializeComponent();

	UpdatePawnOwner();

	K2_UpdatePawnOwner();  // PostLoad() will crash
}

void UPushPawnComponent::PostLoad()
{
	Super::PostLoad();

	// Required for programmatically changing defaults during editor time
	UpdatePawnOwner();
}

void UPushPawnComponent::UpdatePawnOwner()
{
	PawnOwner = Cast<APawn>(GetOwner());
	CharacterOwner = Cast<ACharacter>(GetOwner());
}
