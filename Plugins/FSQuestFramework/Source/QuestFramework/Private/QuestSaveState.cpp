// Copyright, Sami Kangasmaa 2022


#include "QuestSaveState.h"
#include "QuestFrameworkModule.h"
#include "QuestContext.h"
#include "Quest.h"

bool FQuestContextSaveStateData::StoreQuestContext(const UQuestContext* Context)
{
	if (!IsValid(Context))
	{
		UE_LOG(LogQuestFramework, Error, TEXT("Can't store quest context to save state because the context was invalid!"));
		return false;
	}

	if (!Context->IsInitialized())
	{
		UE_LOG(LogQuestFramework, Error, TEXT("Can't store quest context (%s) to save state because it is not initialized!"), *Context->GetFName().ToString());
		return false;
	}

	const UQuest* QuestAsset = Context->GetQuestAsset();
	if (!ensure(QuestAsset))
	{
		return false;
	}

	// Store guid of the context and current quest state
	Guid = Context->GetGuid();
	ResultState = Context->GetQuestState();

	// Save current node as GUID in case that quest is later modified, so change in the indices would not affect
	const int32 NodeIndex = Context->GetActiveNodeIndex();
	const TArray<UQuestNode*>& Nodes = QuestAsset->GetQuestNodes();
	if (!ensure(Nodes.IsValidIndex(NodeIndex)))
	{
		return false;
	}
	check(Nodes[NodeIndex]);
	NodeGuid = Nodes[NodeIndex]->GetNodeGuid();
	
	// Store memory block copy
	const UQuestMemory* MemoryObject = Context->GetQuestMemory();
	check(MemoryObject);
	MemoryBlock = MemoryObject->GetMemoryBlock();

	UE_LOG(LogQuestFramework, Log, TEXT("Stored quest context (%s) to save state."), *Context->GetFName().ToString());

	return true;
}

void FQuestContextSaveStateData::RestoreQuestContext(UQuestContext* Context) const
{
	if (!IsValid(Context))
	{
		UE_LOG(LogQuestFramework, Error, TEXT("Can't restore quest context from save state because the context was invalid!"));
		return;
	}

	if (!Context->IsInitialized())
	{
		UE_LOG(LogQuestFramework, Error, TEXT("Can't restore quest context (%s) from save state because it is not initialized!"), *Context->GetFName().ToString());
		return;
	}

	const UQuest* QuestAsset = Context->GetQuestAsset();
	if (!ensure(QuestAsset))
	{
		return;
	}

	// Find node index
	int32 NodeIndex = -1;
	const TArray<UQuestNode*>& Nodes = QuestAsset->GetQuestNodes();
	for (UQuestNode* Node : Nodes)
	{
		check(Node);
		if (Node->GetNodeGuid() == NodeGuid)
		{
			NodeIndex = Node->GetNodeIndex();
		}
	}

	if (NodeIndex == -1)
	{
		UE_LOG(LogQuestFramework, Error, TEXT("Could not find node to restore (%s) from save state! Starting from root!"), *Context->GetFName().ToString());
		NodeIndex = 0;
	}

	check(Nodes.IsValidIndex(NodeIndex));

	UE_LOG(LogQuestFramework, Log, TEXT("Restoring quest context (%s) from save state..."), *Context->GetFName().ToString());
	Context->RestoreState(Guid, NodeIndex, ResultState, MemoryBlock);
}

// ------------------------------------------------------ //

void FQuestContextSaveState::PrepareWrite(const UQuest* QuestAsset)
{
	ContextStates.Empty();

	if (!IsValid(QuestAsset))
	{
		UE_LOG(LogQuestFramework, Error, TEXT("Can't prepare write of QuestContextSaveState because quest asset was invalid!"));
		return;
	}

	QuestGuid = QuestAsset->GetGuid();
	if (!QuestGuid.IsValid())
	{
		UE_LOG(LogQuestFramework, Error, TEXT("Can't prepare write of QuestContextSaveState because quest asset (%s) has invalid GUID!"), *QuestAsset->GetFName().ToString());
		return;
	}
}

void FQuestContextSaveState::WriteContext(const UQuestContext* Context)
{
	if (!IsValid(Context))
	{
		UE_LOG(LogQuestFramework, Error, TEXT("Can't write context to QuestContextSaveState because context was invalid!"));
		return;
	}

	if (!QuestGuid.IsValid())
	{
		UE_LOG(LogQuestFramework, Error, TEXT("Can't write context %s to QuestContextSaveState because quest guid is missing!"), *Context->GetFName().ToString());
		return;
	}

	const UQuest* QuestAsset = Context->GetQuestAsset();
	if (!ensure(QuestAsset))
	{
		return;
	}

	// Check for missmatch
	if (!ensure(QuestGuid == QuestAsset->GetGuid()))
	{
		return;
	}

	FQuestContextSaveStateData ContextState = FQuestContextSaveStateData();
	if (ContextState.StoreQuestContext(Context))
	{
		ContextStates.Add(ContextState);
	}
}

// ------------------------------------------------------ //

