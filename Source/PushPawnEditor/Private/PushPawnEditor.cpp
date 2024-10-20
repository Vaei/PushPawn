#include "PushPawnEditor.h"

#include "PropertyEditorModule.h"
#include "PushPawnAbilityDetailsCustomization.h"
#include "Abilities/PushPawn_Ability.h"

#define LOCTEXT_NAMESPACE "FPushPawnEditorModule"

void FPushPawnEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	// Anim Instance
	PropertyModule.RegisterCustomClassLayout(
		UPushPawn_Ability::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FPushPawnAbilityDetailsCustomization::MakeInstance)
	);
}

void FPushPawnEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule* PropertyModule = FModuleManager::Get().GetModulePtr<FPropertyEditorModule>("PropertyEditor");
		PropertyModule->UnregisterCustomClassLayout(UPushPawn_Ability::StaticClass()->GetFName());
	}
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FPushPawnEditorModule, PushPawnEditor)