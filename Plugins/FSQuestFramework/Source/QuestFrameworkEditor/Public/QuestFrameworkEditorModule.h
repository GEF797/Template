// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "EdGraphUtilities.h"
#include "AssetTypeCategories.h"
#include "IAssetTypeActions.h"

DECLARE_LOG_CATEGORY_EXTERN(LogQuestFrameworkEditor, All, All)

class FQuestFrameworkEditorModule : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

	// Custom asset category
	EAssetTypeCategories::Type AssetCategory = EAssetTypeCategories::Type::Misc;

	// List of all actions that were registered on StartupModule
	TArray<TSharedPtr<IAssetTypeActions>> AssetTypeActions;

	// Factories for slate widgets
	TSharedPtr<FGraphPanelPinFactory> QuestGraphPinFactory;
	TSharedPtr<FGraphPanelNodeFactory> QuestGraphNodeFactory;

	// Registered details
	TArray<FName> CustomClassLayouts;
	TArray<FName> CustomPropertyLayouts;

private:

	void RegisterCustomDetails();
	void UnregisterCustomDetails();
};
