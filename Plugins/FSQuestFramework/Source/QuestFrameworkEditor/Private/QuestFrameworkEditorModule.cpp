// Copyright Sami Kangasmaa, 2022

#include "QuestFrameworkEditorModule.h"

#include "AssetRegistry/AssetRegistryModule.h"

#include "AssetActions/AssetTypeActions_Quest.h"
#include "AssetActions/AssetTypeActions_QuestObjects.h"

#include "Factory/QuestFactory.h"
#include "Factory/QuestGraphFactory.h"

#include "PropertyEditorDelegates.h"
#include "Details/Details_Quest.h"
#include "Details/Details_QuestGraphNode.h"
#include "Details/Details_QuestObjective.h"
#include "Details/Details_QuestText.h"
#include "Details/Details_QuestVariable.h"

#include "Nodes/QuestGraphNodeBase.h"
#include "Nodes/QuestGraphNode_Root.h"
#include "Nodes/QuestGraphNode_Selector.h"
#include "Nodes/QuestGraphNode_Connection.h"
#include "Nodes/QuestGraphNode_End.h"
#include "Nodes/QuestGraphNode_Stage.h"

#include "Nodes/QuestNode_Root.h"
#include "Nodes/QuestNode_Selector.h"
#include "Nodes/QuestNode_End.h"
#include "Nodes/QuestNode_Stage.h"

#include "QuestMemory.h"
#include "QuestManager.h"

#include "QuestFrameworkSettings.h"


#define LOCTEXT_NAMESPACE "QuestFrameworkEditor"

DEFINE_LOG_CATEGORY(LogQuestFrameworkEditor)

namespace QuestFrameworkEditorModuleUtils
{
	template<typename TAssetActionType> 
	void RegisterAssetTypeAction(IAssetTools& AssetTools, TArray<TSharedPtr<IAssetTypeActions>>& ActionList, EAssetTypeCategories::Type AssetCategory)
	{
		TSharedRef<TAssetActionType> AssetAction = MakeShared<TAssetActionType>(AssetCategory);
		AssetTools.RegisterAssetTypeActions(AssetAction);
		ActionList.Add(AssetAction);
	}
}

void FQuestFrameworkEditorModule::StartupModule()
{
	FQuestEditorStyles::InitializeSlateStyles();

	// Register asset actions
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get();
	AssetCategory = AssetTools.RegisterAdvancedAssetCategory(TEXT("Quest Framework"), NSLOCTEXT("QuestFrameworkEditor", "QuestAssetCategory", "Quest Framework"));

	QuestFrameworkEditorModuleUtils::RegisterAssetTypeAction<FAssetTypeActions_Quest>(AssetTools, AssetTypeActions, AssetCategory);
	QuestFrameworkEditorModuleUtils::RegisterAssetTypeAction<FAssetTypeActions_QuestCondition>(AssetTools, AssetTypeActions, AssetCategory);
	QuestFrameworkEditorModuleUtils::RegisterAssetTypeAction<FAssetTypeActions_QuestEvent>(AssetTools, AssetTypeActions, AssetCategory);
	QuestFrameworkEditorModuleUtils::RegisterAssetTypeAction<FAssetTypeActions_QuestService>(AssetTools, AssetTypeActions, AssetCategory);
	QuestFrameworkEditorModuleUtils::RegisterAssetTypeAction<FAssetTypeActions_QuestObjectiveObserver>(AssetTools, AssetTypeActions, AssetCategory);
	QuestFrameworkEditorModuleUtils::RegisterAssetTypeAction<FAssetTypeActions_QuestTextArgument>(AssetTools, AssetTypeActions, AssetCategory);
	QuestFrameworkEditorModuleUtils::RegisterAssetTypeAction<FAssetTypeActions_QuestCustomData>(AssetTools, AssetTypeActions, AssetCategory);

	RegisterCustomDetails();

	// Register slate factories
	QuestGraphPinFactory = MakeShared<FQuestGraphPinFactory>();
	FEdGraphUtilities::RegisterVisualPinFactory(QuestGraphPinFactory);

	QuestGraphNodeFactory = MakeShared<FQuestGraphNodeFactory>();
	FEdGraphUtilities::RegisterVisualNodeFactory(QuestGraphNodeFactory);

	// Load settings
	const UQuestFrameworkSettings* Settings = GetDefault<UQuestFrameworkSettings>();
}

void FQuestFrameworkEditorModule::ShutdownModule()
{
	const FModuleManager& ModuleManger = FModuleManager::Get();

	UnregisterCustomDetails();

	// Unregister asset actions
	if (ModuleManger.IsModuleLoaded(TEXT("AssetTools")))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get();
		for (auto AssetAction : AssetTypeActions)
		{
			AssetTools.UnregisterAssetTypeActions(AssetAction.ToSharedRef());
		}
	}
	AssetTypeActions.Empty();

	// Unregister slate factories
	if (QuestGraphPinFactory.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualPinFactory(QuestGraphPinFactory);
	}
	if (QuestGraphNodeFactory.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualNodeFactory(QuestGraphNodeFactory);
	}

	FQuestEditorStyles::UninitializeSlateStyles();
}

void FQuestFrameworkEditorModule::RegisterCustomDetails()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));

	TArray<FOnGetDetailCustomizationInstance> ClassLayouts;

	// Quest asset
	ClassLayouts.Add(FOnGetDetailCustomizationInstance::CreateStatic(&FDetails_Quest::MakeInstance));
	CustomClassLayouts.Add(UQuest::StaticClass()->GetFName());

	// Quest graph nodes
	ClassLayouts.Add(FOnGetDetailCustomizationInstance::CreateStatic(&FDetails_QuestGraphNode::MakeInstance));
	CustomClassLayouts.Add(UQuestGraphNode_Root::StaticClass()->GetFName());

	ClassLayouts.Add(FOnGetDetailCustomizationInstance::CreateStatic(&FDetails_QuestGraphNode::MakeInstance));
	CustomClassLayouts.Add(UQuestGraphNode_Stage::StaticClass()->GetFName());

	ClassLayouts.Add(FOnGetDetailCustomizationInstance::CreateStatic(&FDetails_QuestGraphNode::MakeInstance));
	CustomClassLayouts.Add(UQuestGraphNode_Selector::StaticClass()->GetFName());

	ClassLayouts.Add(FOnGetDetailCustomizationInstance::CreateStatic(&FDetails_QuestGraphNode::MakeInstance));
	CustomClassLayouts.Add(UQuestGraphNode_End::StaticClass()->GetFName());

	ClassLayouts.Add(FOnGetDetailCustomizationInstance::CreateStatic(&FDetails_QuestGraphNode_Connection::MakeInstance));
	CustomClassLayouts.Add(UQuestGraphNode_Connection::StaticClass()->GetFName());


	check(ClassLayouts.Num() == CustomClassLayouts.Num());
	for (int32 Idx = 0; Idx < CustomClassLayouts.Num(); Idx++)
	{
		PropertyModule.RegisterCustomClassLayout(CustomClassLayouts[Idx], ClassLayouts[Idx]);
	}

	TArray<FOnGetPropertyTypeCustomizationInstance> PropertyLayouts;

	// Quest text
	PropertyLayouts.Add(FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FDetails_QuestText::MakeInstance));
	CustomPropertyLayouts.Add(FQuestText::StaticStruct()->GetFName());

	// Quest text arg
	PropertyLayouts.Add(FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FDetails_QuestTextArg::MakeInstance));
	CustomPropertyLayouts.Add(FQuestTextArg::StaticStruct()->GetFName());

	// Quest variable
	PropertyLayouts.Add(FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FDetails_QuestVariable::MakeInstance));
	CustomPropertyLayouts.Add(FQuestVariable::StaticStruct()->GetFName());

	// Variable comparison
	PropertyLayouts.Add(FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FDetails_QuestVariableComparison::MakeInstance));
	CustomPropertyLayouts.Add(FQuestVariableComparison::StaticStruct()->GetFName());

	// Variable initializer
	PropertyLayouts.Add(FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FDetails_QuestVariableInitializer::MakeInstance));
	CustomPropertyLayouts.Add(FQuestVariableInitializer::StaticStruct()->GetFName());

	// Variable bounds
	PropertyLayouts.Add(FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FDetails_QuestVariableBounds::MakeInstance));
	CustomPropertyLayouts.Add(FQuestVariableBounds::StaticStruct()->GetFName());

	// Stage objectives
	PropertyLayouts.Add(FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FDetails_QuestStageObjectives::MakeInstance));
	CustomPropertyLayouts.Add(FQuestStageObjectives::StaticStruct()->GetFName());

	check(PropertyLayouts.Num() == CustomPropertyLayouts.Num());
	for (int32 Idx = 0; Idx < CustomPropertyLayouts.Num(); Idx++)
	{
		PropertyModule.RegisterCustomPropertyTypeLayout(CustomPropertyLayouts[Idx], PropertyLayouts[Idx]);
	}

	PropertyModule.NotifyCustomizationModuleChanged();
}

void FQuestFrameworkEditorModule::UnregisterCustomDetails()
{
	const FModuleManager& ModuleManger = FModuleManager::Get();
	if (ModuleManger.IsModuleLoaded(TEXT("PropertyEditor")))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
		for (int32 Idx = 0; Idx < CustomClassLayouts.Num(); Idx++)
		{
			PropertyModule.UnregisterCustomClassLayout(CustomClassLayouts[Idx]);
		}

		for (int32 Idx = 0; Idx < CustomPropertyLayouts.Num(); Idx++)
		{
			PropertyModule.UnregisterCustomPropertyTypeLayout(CustomPropertyLayouts[Idx]);
		}
	}
	CustomClassLayouts.Empty();
	CustomPropertyLayouts.Empty();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FQuestFrameworkEditorModule, QuestFrameworkEditor)