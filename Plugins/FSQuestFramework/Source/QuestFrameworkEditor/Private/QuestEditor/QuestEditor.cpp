// Copyright Sami Kangasmaa, 2022


#include "QuestEditor/QuestEditor.h"
#include "Editor.h"

#include "PropertyEditorModule.h"
#include "GraphEditorActions.h"
#include "EdGraphUtilities.h"
#include "Framework/Commands/GenericCommands.h"
#include "Kismet2/BlueprintEditorUtils.h"

#include "Graph/QuestGraph.h"
#include "Graph/QuestGraphSchema.h"

#include "Nodes/QuestGraphNodeBase.h"
#include "Nodes/QuestGraphNode_Root.h"
#include "Nodes/QuestGraphNode_Selector.h"
#include "Nodes/QuestGraphNode_Connection.h"
#include "Nodes/QuestGraphNode_End.h"
#include "Nodes/QuestGraphNode_Stage.h"

#include "QuestEditor/QuestEditorUtils.h"
#include "QuestEditor/Slate/SQuestVariableViewWidget.h"

#include "Compiler/QuestCompiler.h"

#define LOCTEXT_NAMESPACE "QuestEditor"

const FName FQuestEditor::GraphCanvasTabID(TEXT("QuestEditor_GraphCanvas"));
const FName FQuestEditor::InspectorTabID(TEXT("QuestEditor_Details"));
const FName FQuestEditor::VariablesTabID(TEXT("QuestEditor_Variables"));

FQuestEditor::FQuestEditor()
	: IQuestEditor()
{
	QuestAsset = nullptr;
	GEditor->RegisterForUndo(this);
}

FQuestEditor::~FQuestEditor()
{
	GEditor->UnregisterForUndo(this);

	// Reset compiler access when editor closes
	if (QuestAsset)
	{
		QuestAsset->SetCompilerAccess(nullptr);
	}
}

void FQuestEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(QuestAsset);
}

void FQuestEditor::InitQuestEditor(EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UQuest* Quest)
{
	// Assign quest asset that we are editing
	QuestAsset = Quest;

	// Set compiler access for the quest before starting to edit it
	QuestAsset->SetCompilerAccess(this);

	// Undo/redo requires this
	QuestAsset->SetFlags(RF_Transactional);

	// Create or restore graph in quest
	RestoreQuestGraph();

	// Register all commands
	FGraphEditorCommands::Register();

	// Bind registered commands to implement them
	BindEditorCommands();

	// Create widgets for quest editing
	CreateInternalWidgets();

	// Create slate layout
	const TSharedRef<FTabManager::FLayout> QuestEditorMainLayout =
		FTabManager::NewLayout("Standalone_QuestEditor_Layout_v1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				// Toolbar
				FTabManager::NewStack()
				->SetSizeCoefficient(0.1f)
				->SetHideTabWell(true)
			)
			->Split
			(
				// Main Application area
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Horizontal)
				->SetSizeCoefficient(0.9f)
				->Split
				(
					// Variables at left
					FTabManager::NewStack()
					->SetSizeCoefficient(0.125f)
					->SetHideTabWell(true)
					->AddTab(VariablesTabID, ETabState::OpenedTab)
				)
				->Split
				(
					// Graph canvas at middle
					FTabManager::NewStack()
					->SetSizeCoefficient(0.625f)
					->SetHideTabWell(true)
					->AddTab(GraphCanvasTabID, ETabState::OpenedTab)
				)
				->Split
				(
					// Inspector at right
					FTabManager::NewStack()
					->SetSizeCoefficient(0.25f)
					->SetHideTabWell(true)
					->AddTab(InspectorTabID, ETabState::OpenedTab)
				)
			)
		);

	FAssetEditorToolkit::InitAssetEditor(Mode, InitToolkitHost, TEXT("QuestEditorAppName"), QuestEditorMainLayout, true, true, QuestAsset, false);

	RegenerateMenusAndToolbars();
}

void FQuestEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = TabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenuQuestEditor", "Quest Editor"));
	const TSharedRef<FWorkspaceItem> WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(InspectorTabID, FOnSpawnTab::CreateSP(this, &Self::SpawnTab_Inspector))
		.SetDisplayName(LOCTEXT("InspectorTabLabel", "Inspector"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(GraphCanvasTabID, FOnSpawnTab::CreateSP(this, &Self::SpawnTab_GraphCanvas))
		.SetDisplayName(LOCTEXT("GraphCanvasTabLabel", "Quest Graph"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.EventGraph_16x"));

	InTabManager->RegisterTabSpawner(VariablesTabID, FOnSpawnTab::CreateSP(this, &Self::SpawnTab_Variables))
		.SetDisplayName(LOCTEXT("VariablesTabLabel", "Variables"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));

}

void FQuestEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
	
	InTabManager->UnregisterTabSpawner(GraphCanvasTabID);
	InTabManager->UnregisterTabSpawner(InspectorTabID);
	InTabManager->UnregisterTabSpawner(VariablesTabID);
}

FText FQuestEditor::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "Quest Editor");
}

FText FQuestEditor::GetToolkitName() const
{
	if (QuestAsset != nullptr)
	{
		return FAssetEditorToolkit::GetLabelForObject(QuestAsset);
	}
	return FText();
}

void FQuestEditor::SaveAsset_Execute()
{
	FAssetEditorToolkit::SaveAsset_Execute();
}

void FQuestEditor::SaveAssetAs_Execute()
{
	FAssetEditorToolkit::SaveAssetAs_Execute();
}

void FQuestEditor::PostUndo(bool bSuccess)
{
	if (bSuccess)
	{
		if (GraphEditorWidget.IsValid())
		{
			GraphEditorWidget->ClearSelectionSet();
		}
		RefreshWidgets();
		RefreshInspector();
		FSlateApplication::Get().DismissAllMenus();
	}
}

void FQuestEditor::PostRedo(bool bSuccess)
{
	if (bSuccess)
	{
		if (GraphEditorWidget.IsValid())
		{
			GraphEditorWidget->ClearSelectionSet();
		}
		RefreshWidgets();
		RefreshInspector();
		FSlateApplication::Get().DismissAllMenus();
	}
}

void FQuestEditor::RestoreQuestGraph()
{
	check(QuestAsset);

	UQuestGraph* MyGraph = Cast<UQuestGraph>(QuestAsset->HasGraph() ? QuestAsset->GetGraph() : nullptr);
	const bool bNewGraph = MyGraph == nullptr;

	if (MyGraph == nullptr)
	{
		QuestAsset->SetGraph(FBlueprintEditorUtils::CreateNewGraph(QuestAsset, TEXT("Quest"), UQuestGraph::StaticClass(), UQuestGraphSchema::StaticClass()));
		MyGraph = Cast<UQuestGraph>(QuestAsset->GetGraph());

		const UEdGraphSchema* Schema = MyGraph->GetSchema();
		Schema->CreateDefaultNodesForGraph(*MyGraph);
	}
}

void FQuestEditor::CompileQuest()
{
	if (QuestAsset)
	{
		FQuestCompiler Compiler(QuestAsset);
		Compiler.CompileQuest();

		RefreshWidgets();
	}
}

void FQuestEditor::CompileQuestInEditorModule(UQuest* Quest)
{
	if (Quest && QuestAsset == Quest)
	{
		CompileQuest();
	}
}

void FQuestEditor::BindEditorCommands()
{
	if (GraphEditorCommands.IsValid())
	{
		return;
	}
	GraphEditorCommands = MakeShared<FUICommandList>();

	GraphEditorCommands->MapAction(
		FGenericCommands::Get().Delete,
		FExecuteAction::CreateSP(this, &Self::OnCommandDeleteNodes),
		FCanExecuteAction::CreateSP(this, &Self::CanDeleteSelectedNodes)
	);
}

void FQuestEditor::CreateInternalWidgets()
{
	GraphEditorWidget = CreateGraphEditorWidget();

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bUpdatesFromSelection = false;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.NotifyHook = this;
	DetailsViewArgs.bHideSelectionTip = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::ObjectsUseNameArea;
	DetailsViewArgs.bAllowSearch = true;

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
	InspectorView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	InspectorView->SetObject(QuestAsset);

	VariablesTabWidget = CreateVariablesWidget();
	RefreshVariableTab();
}

TSharedRef<SGraphEditor> FQuestEditor::CreateGraphEditorWidget()
{
	check(QuestAsset);

	FGraphAppearanceInfo AppearanceInfo;
	AppearanceInfo.CornerText = LOCTEXT("AppearanceCornerText", "QUEST");

	// Bind graph events
	SGraphEditor::FGraphEditorEvents InEvents;
	InEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FQuestEditor::OnSelectedNodesChanged);
	InEvents.OnTextCommitted = FOnNodeTextCommitted::CreateSP(this, &FQuestEditor::OnNodeTitleCommitted);

	return SNew(SGraphEditor)
		.AdditionalCommands(GraphEditorCommands)
		.IsEditable(true)
		.Appearance(AppearanceInfo)
		.GraphToEdit(QuestAsset->GetGraph())
		.GraphEvents(InEvents)
		.ShowGraphStateOverlay(false);
}

TSharedRef<SQuestVariableViewWidget> FQuestEditor::CreateVariablesWidget()
{
	return SNew(SQuestVariableViewWidget);
}

void FQuestEditor::OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged) const
{
}

void FQuestEditor::OnSelectedNodesChanged(const TSet<UObject*>& NewSelection)
{
	TArray<UObject*> DetailsSelection;
	if (NewSelection.Num() <= 0)
	{
		DetailsSelection.Add(QuestAsset);
	}
	else
	{
		for (UObject* Selected : NewSelection)
		{
			ensure(Selected);
			DetailsSelection.Add(Selected);
		}
	}

	if (InspectorView.IsValid())
	{
		InspectorView->SetObjects(DetailsSelection, true);
	}
}

void FQuestEditor::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged)
{
	if (PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		if (GraphEditorWidget.IsValid())
		{
			GraphEditorWidget->NotifyGraphChanged();
		}
		RefreshVariableTab();
	}
}

void FQuestEditor::OnCommandDeleteNodes() const
{
	const FScopedTransaction Transaction(LOCTEXT("DeleteSelectedNodes", "Quest Editor: Delete Node"));
	QuestAsset->Modify();

	check(GraphEditorWidget);
	TSet<UObject*> SelectedNodeObjectSet = GraphEditorWidget->GetSelectedNodes();

	UQuestGraph* QuestGraph = CastChecked<UQuestGraph>(QuestAsset->GetGraph());
	QuestGraph->Modify();

	if (GraphEditorWidget.IsValid())
	{
		GraphEditorWidget->ClearSelectionSet();
	}

	bool bDeletedAny = false;
	auto DeleteGraphNode = [&bDeletedAny, QuestGraph](UEdGraphNode* Node)
	{
		if (QuestGraph->Modify())
		{
			if (Node->Modify())
			{
				if (QuestGraph->RemoveQuestGraphNode(Node))
				{
					bDeletedAny = true;
				}
			}
		}
	};

	for (UObject* NodeObject : SelectedNodeObjectSet)
	{
		UEdGraphNode* Node = CastChecked<UEdGraphNode>(NodeObject);

		if (!Node->CanUserDeleteNode())
		{
			continue;
		}

		if (UQuestGraphNode_Root* Root = Cast<UQuestGraphNode_Root>(Node))
		{
			continue; // Root cant be deleted
		}
		else if (UQuestGraphNodeBase* BaseNode = Cast<UQuestGraphNodeBase>(Node))
		{
			if(UQuestGraphNode_Connection* ConnectionNode = Cast<UQuestGraphNode_Connection>(BaseNode))
			{
				DeleteGraphNode(ConnectionNode);
			}
			else if (UQuestGraphNodeCommonBase* CommonBase = Cast<UQuestGraphNodeCommonBase>(BaseNode))
			{
				// Also remove connections
				TArray<UQuestGraphNode_Connection*> Connections = CommonBase->GetConnectionsToParents();
				Connections.Append(CommonBase->GetConnectionsToChildren());

				for (UQuestGraphNode_Connection* Connection : Connections)
				{
					DeleteGraphNode(Connection);
				}

				DeleteGraphNode(CommonBase);
			}
			else
			{
				checkNoEntry();
			}
		}
		else
		{
			DeleteGraphNode(Node);
		}
	}

	if (bDeletedAny)
	{
		QuestAsset->PostEditChange();
		QuestAsset->MarkPackageDirty();
		RefreshWidgets();
		RefreshInspector();
	}
}

bool FQuestEditor::CanDeleteSelectedNodes() const
{
	check(GraphEditorWidget);
	TSet<UObject*> SelectedNodeObjectSet = GraphEditorWidget->GetSelectedNodes();

	// Check that deletion set does not contain root node
	for (const UObject* Node : SelectedNodeObjectSet)
	{
		if (Node && Node->IsA(UQuestGraphNode_Root::StaticClass()))
		{
			return false;
		}
	}

	return SelectedNodeObjectSet.Num() > 0;
}

void FQuestEditor::RefreshWidgets() const
{
	if (GraphEditorWidget.IsValid())
	{
		GraphEditorWidget->NotifyGraphChanged();
	}

	RefreshVariableTab();
}

void FQuestEditor::RefreshInspector() const
{
	if (InspectorView.IsValid())
	{
		if (QuestAsset)
		{
			InspectorView->SetObject(QuestAsset, true);
		}
		else
		{
			InspectorView->ForceRefresh();
		}
		InspectorView->ClearSearch();
	}
}

void FQuestEditor::RefreshVariableTab() const
{
	if (VariablesTabWidget.IsValid())
	{
		// Create quest info
		TArray<FQuestVariableInfo> VariableInfoArray = FQuestEditorUtils::CreateQuestVariableInfoData(QuestAsset);
		VariablesTabWidget->UpdateVariableList(VariableInfoArray);
	}
}

TSharedRef<SDockTab> FQuestEditor::SpawnTab_GraphCanvas(const FSpawnTabArgs& Args) const
{
	return SNew(SDockTab).Label(LOCTEXT("QuestGraphTitle", "Quest Graph"))[GraphEditorWidget.ToSharedRef()];
}

TSharedRef<SDockTab> FQuestEditor::SpawnTab_Inspector(const FSpawnTabArgs& Args) const
{
	check(Args.GetTabId() == InspectorTabID);

	TSharedRef<SDockTab> Tab = SNew(SDockTab)
		.Label(LOCTEXT("QuestEditorInspectorTitle", "Inspector"))
		.TabColorScale(GetTabColorScale())
		[
			InspectorView.ToSharedRef()
		];

	const FSlateBrush* IconBrush = FAppStyle::GetBrush(TEXT("GenericEditor.Tabs.Properties"));
	Tab->SetTabIcon(IconBrush);

	return Tab;
}

TSharedRef<SDockTab> FQuestEditor::SpawnTab_Variables(const FSpawnTabArgs& Args) const
{
	check(Args.GetTabId() == VariablesTabID);

	TSharedRef<SDockTab> Tab = SNew(SDockTab)
		.Label(LOCTEXT("QuestEditorVariablesTitle", "Variables"))
		.TabColorScale(GetTabColorScale())
		[
			VariablesTabWidget.ToSharedRef()
		];

	const FSlateBrush* IconBrush = FAppStyle::GetBrush(TEXT("GenericEditor.Tabs.Properties"));
	Tab->SetTabIcon(IconBrush);
	return Tab;
}

#undef LOCTEXT_NAMESPACE