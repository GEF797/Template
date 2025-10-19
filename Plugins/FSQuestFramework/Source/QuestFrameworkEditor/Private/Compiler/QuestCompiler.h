// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "Kismet2/CompilerResultsLog.h"

#include "Quest.h"

#include "Nodes/QuestGraphNode_Connection.h"
#include "Nodes/QuestGraphNode_End.h"
#include "Nodes/QuestGraphNode_Root.h"
#include "Nodes/QuestGraphNode_Stage.h"
#include "Nodes/QuestGraphNode_Selector.h"

#include "Nodes/QuestNode_End.h"
#include "Nodes/QuestNode_Root.h"
#include "Nodes/QuestNode_Selector.h"
#include "Nodes/QuestNode_Stage.h"

#include "Graph/QuestGraph.h"


/**
 * Compiles quest from graph nodes into runtime format
 */
class FQuestCompiler
{
private:

	FQuestCompiler() {}

public:

	FQuestCompiler(UQuest* Quest);
	~FQuestCompiler();

public:

	/*
	* Executes full compilation of quest
	*/
	void CompileQuest();

	/**
	 * Runs through node hierarchy in graph and assigns node index for each node
	 */
	void CompileNodeIndicies();

private:

	UQuest* QuestAsset = nullptr;

	// Next index to provide for a node when requested
	int32 NextNodeIndex = 0;

	// Provides next node index available
	int32 GetNextNodeIndex()
	{
		const int32 NextIndex = NextNodeIndex;
		NextNodeIndex++;
		return NextIndex;
	}

	// All nodes that are connected to the root node
	TSet<UQuestGraphNodeCommonBase*> ConnectedNodes;

	// Solves list of nodes that are linked to root node
	void SolveRootConnectedNodes();

	// Recursive solver for nodes connected to node (All children of given node).
	void SolveConnectedNodes(UQuestGraphNodeCommonBase* Node);

	// Solves path to root recursively
	bool SolvePathToRoot(UQuestGraphNodeCommonBase* Node, TArray<UQuestGraphNodeCommonBase*>& Output, TSet<UQuestGraphNodeCommonBase*>& VisitedNodes) const;

	// Mapping for node depths (how "far" away from root)
	TMap<UQuestGraphNodeCommonBase*, int32> NodeDepths;

	// Solves NodeDepths map
	void SolveNodeDepths();

	TArray<UQuestGraphNodeCommonBase*> GetNodesInDepth(int32 Depth) const;

	// Sorts children nodes by x location of nodes (first one in the output is leftmost)
	TArray<UQuestGraphNodeCommonBase*> SortChildren(const TArray<UQuestGraphNodeCommonBase*>& Children) const;

	// Same as SortChildren but gets children from node to sort them
	TArray<UQuestGraphNodeCommonBase*> GetAndSortNodeChildren(UQuestGraphNodeCommonBase* Node) const;

	TSet<UQuestGraphNodeCommonBase*> IndexGivenNodes;

	// Solves and assigns index for each connected node
	void SolveNodeIndicies();

	// List of result nodes. This is set to quest after compilation
	UPROPERTY()
	TArray<UQuestNode*> ResultNodes;

	// Compiles all connected nodes
	void CompileNodes();

	// Compiles conditions from connections to selector node
	void CompileSelector(UQuestGraphNode_Selector* GraphNode, UQuestNode_Selector* Selector, const TMap<int32, UQuestGraphNodeCommonBase*>& NodeTable);

	// Clears all error texts in nodes
	void ClearNodeErrors();

	// Validates that connected graph nodes are okay before commiting into compilation
	bool ValidateConnectedGraphNodes();

	// Validates that compilation was succesfull before storing results.
	bool ValidateCompilation();
};
