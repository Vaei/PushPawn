// Copyright (c) Jared Taylor. All Rights Reserved


#include "Components/PusheeComponent.h"

#include "GameFramework/Character.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PusheeComponent)

UPusheeComponent::UPusheeComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UPusheeComponent::OnRegister()
{
	Super::OnRegister();

	if (GetWorld() && GetWorld()->IsGameWorld())
	{
		UpdatePawnOwner();
	}
}

void UPusheeComponent::InitializeComponent()
{
	Super::InitializeComponent();

	UpdatePawnOwner();
}

void UPusheeComponent::PostLoad()
{
	Super::PostLoad();

	// Required for programmatically changing defaults during editor time
	UpdatePawnOwner();
}

void UPusheeComponent::UpdatePawnOwner()
{
	PawnOwner = Cast<APawn>(GetOwner());
	CharacterOwner = Cast<ACharacter>(GetOwner());
}
