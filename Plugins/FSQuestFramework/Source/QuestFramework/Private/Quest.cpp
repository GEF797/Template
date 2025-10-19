// Copyright Sami Kangasmaa, 2022


#include "Quest.h"
#include "QuestFrameworkModule.h"

#include "Nodes/QuestNode_Root.h"
#include "Nodes/QuestNode_Stage.h"

void UQuest::PostLoad()
{
	Super::PostLoad();

	if (!HasValidGuid())
	{
		GenerateGuid();
	}
}

void UQuest::PostInitProperties()
{
	Super::PostInitProperties();

	if (HasAnyFlags(RF_ClassDefaultObject | RF_NeedLoad))
	{
		return;
	}

	// Regenerate guid if it was not set
	if (!HasValidGuid())
	{
		GenerateGuid();
	}
}

void UQuest::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);
	GenerateGuid();
}

void UQuest::PostEditImport()
{
	Super::PostEditImport();
	GenerateGuid();
}

void UQuest::GenerateGuid()
{
	QuestGuid = FGuid::NewGuid();
}

UQuestNode_Root* UQuest::GetRootNode() const
{
	if (Nodes.Num() <= 0)
	{
		return nullptr;
	}

	// Iterate nodes and return first root node found. root node should be in first index
	for (UQuestNode* Node : Nodes)
	{
		if (UQuestNode_Root* RootNode = Cast<UQuestNode_Root>(Node))
		{
			return RootNode;
		}
	}

	return nullptr;
}

void UQuest::GetAllStageNodesNames(TArray<FName>& OutNames) const
{
	for (UQuestNode* Node : Nodes)
	{
		if (UQuestNode_Stage* StageNode = Cast<UQuestNode_Stage>(Node))
		{
			OutNames.Add(StageNode->GetLogicalName());
		}
	}
}

UQuestNode_Stage* UQuest::FindStageNodeByName(const FName& Name) const
{
	for (UQuestNode* Node : Nodes)
	{
		if (UQuestNode_Stage* StageNode = Cast<UQuestNode_Stage>(Node))
		{
			if (StageNode->GetLogicalName() == Name)
			{
				return StageNode;
			}
		}
	}
	return nullptr;
}

TArray<UQuestService*> UQuest::GetInstancedServices() const
{
	TArray<UQuestService*> OutServices;
	for (UQuestService* Service : Services)
	{
		if (Service && Service->IsInstanced())
		{
			OutServices.Add(Service);
		}
	}
	return OutServices;
}

TArray<UQuestService*> UQuest::GetNonInstancedServices() const
{
	TArray<UQuestService*> OutServices;
	for (UQuestService* Service : Services)
	{
		if (Service && !Service->IsInstanced())
		{
			OutServices.Add(Service);
		}
	}
	return OutServices;
}

/// ------ EDITOR ------------- //
#if WITH_EDITOR

void UQuest::SetGraph(UEdGraph* Graph)
{
	check(Graph);
	ensure(QuestGraph == nullptr);
	QuestGraph = Graph;
}

void UQuest::CompileInEditor()
{
	if (CompilerAccess)
	{
		CompilerAccess->CompileQuestInEditorModule(this);
	}
}

void UQuest::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);

	CompileInEditor();
}

#endif
/// ------ END EDITOR ------------- //