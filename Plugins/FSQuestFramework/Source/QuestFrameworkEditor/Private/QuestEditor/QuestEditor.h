// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWidget.h"
#include "GraphEditor.h"
#include "Misc/NotifyHook.h"
#include "Toolkits/IToolkitHost.h"
#include "EditorUndoClient.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "IDetailsView.h"
#include "EdGraph/EdGraph.h"

#include "QuestEditor/Slate/SQuestVariableViewWidget.h"
#include "QuestEditor/IQuestEditor.h"

#include "Quest.h"


/**
 * Editor to create quest graphs
 */
class FQuestEditor : public IQuestEditor, public FNotifyHook, public FEditorUndoClient, public FGCObject
{
	typedef FQuestEditor Self;

public:

	SLATE_BEGIN_ARGS(Self) {}
	SLATE_END_ARGS()

	FQuestEditor();
	virtual ~FQuestEditor();

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override
	{
		return FString("QuestEditor");
	}

	void InitQuestEditor(EToolkitMode::Type Mode,const TSharedPtr<IToolkitHost>& InitToolkitHost, UQuest* Quest);

public:

	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual FText GetBaseToolkitName() const override;
	virtual FText GetToolkitName() const override;
	virtual FName GetToolkitFName() const override { return FName(TEXT("QuestEditor")); }
	virtual FText GetToolkitToolTipText() const override { return GetToolTipTextForObject(Cast<UObject>(QuestAsset)); }
	virtual FLinearColor GetWorldCentricTabColorScale() const override { return FLinearColor::White; }
	virtual FString GetWorldCentricTabPrefix() const override { return FString(TEXT("QuestEditor")); }

public:

	bool CanSaveAsset() const override { return true; }
	bool CanSaveAssetAs() const override { return true; }
	void SaveAsset_Execute() override;
	void SaveAssetAs_Execute() override;

	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;

public:

	// Restores state of graph being edited. Creates a new one, if none exists
	void RestoreQuestGraph();

	// Compiles quest being edited
	void CompileQuest();

	// Called from quest asset
	virtual void CompileQuestInEditorModule(UQuest* Quest) override;

private:

	void BindEditorCommands();
	void CreateInternalWidgets();
	TSharedRef<SGraphEditor> CreateGraphEditorWidget();
	TSharedRef<SQuestVariableViewWidget> CreateVariablesWidget();

private:

	TSharedPtr<FUICommandList> GraphEditorCommands;

private: // Callbacks

	void OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged) const;

	void OnSelectedNodesChanged(const TSet<UObject*>& NewSelection);

	virtual void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged) override;

	void OnCommandDeleteNodes() const;
	bool CanDeleteSelectedNodes() const;

private:

	// Quest asset we are currently editing
	TObjectPtr<UQuest> QuestAsset;

private: // Slate

	void RefreshWidgets() const;
	void RefreshInspector() const;
	void RefreshVariableTab() const;

	TSharedPtr<SGraphEditor> GraphEditorWidget;
	TSharedPtr<IDetailsView> InspectorView;
	TSharedPtr<SQuestVariableViewWidget> VariablesTabWidget;

	TSharedRef<SDockTab> SpawnTab_GraphCanvas(const FSpawnTabArgs& Args) const;
	TSharedRef<SDockTab> SpawnTab_Inspector(const FSpawnTabArgs& Args) const;
	TSharedRef<SDockTab> SpawnTab_Variables(const FSpawnTabArgs& Args) const;

public:

	static const FName GraphCanvasTabID;
	static const FName InspectorTabID;
	static const FName VariablesTabID;
};
