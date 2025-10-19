// Copyright Sami Kangasmaa, 2022


#include "Compiler/QuestCompiler.h"

FQuestCompiler::FQuestCompiler(UQuest* Quest)
{
	check(Quest);
	QuestAsset = Quest;
}

FQuestCompiler::~FQuestCompiler()
{
	QuestAsset = nullptr;
}

void FQuestCompiler::CompileQuest()
{
	check(QuestAsset);

	ClearNodeErrors();
	CompileNodeIndicies();
	CompileNodes();
}

void FQuestCompiler::CompileNodeIndicies()
{
	check(QuestAsset);

	// First solve all nodes that are connected to the root
	SolveRootConnectedNodes();

	// Solve depths of each node connected to the root
	SolveNodeDepths();

	// Solve and assign node indicies
	SolveNodeIndicies();
}

void FQuestCompiler::SolveRootConnectedNodes()
{
	check(QuestAsset);

	ConnectedNodes.Empty();

	UQuestGraph* Graph = CastChecked<UQuestGraph>(QuestAsset->GetGraph());
	
	// Find root node
	TArray<UQuestGraphNode_Root*> RootNodes = Graph->GetGraphNodes_Root();
	if (!ensure(RootNodes.Num() > 0))
	{
		return;
	}

	UQuestGraphNode_Root* Root = RootNodes[0];
	check(Root);

	ConnectedNodes.Add(Root);
	SolveConnectedNodes(Root);
}

void FQuestCompiler::SolveConnectedNodes(UQuestGraphNodeCommonBase* Node)
{
	check(Node);

	TArray<UQuestGraphNodeCommonBase*> ChildrenNodes = Node->GetChildrenNodes();
	for (UQuestGraphNodeCommonBase* ChildNode : ChildrenNodes)
	{
		if (ConnectedNodes.Contains(ChildNode))
		{
			// Already visited
			continue;
		}
		ConnectedNodes.Add(ChildNode);

		// Solve connected nodes of the child
		SolveConnectedNodes(ChildNode);
	}
}

bool FQuestCompiler::SolvePathToRoot(UQuestGraphNodeCommonBase* Node, TArray<UQuestGraphNodeCommonBase*>& Output, TSet<UQuestGraphNodeCommonBase*>& VisitedNodes) const
{
	if (VisitedNodes.Contains(Node))
	{
		return false;
	}
	VisitedNodes.Add(Node);

	// Root found
	if (Node->IsA<UQuestGraphNode_Root>())
	{
		return true;
	}

	// check parents and add to list if this leads to root
	TArray<UQuestGraphNodeCommonBase*> Parents = Node->GetParentNodes();
	for (UQuestGraphNodeCommonBase* ParentNode : Parents)
	{
		// If root was found from parent chain -> add to output
		if (SolvePathToRoot(ParentNode, Output, VisitedNodes) == true)
		{
			Output.Add(ParentNode);
			return true;
		}
	}

	return false;
}

void FQuestCompiler::SolveNodeDepths()
{
	NodeDepths.Empty();

	// Solve path to root to determine node depth
	for (UQuestGraphNodeCommonBase* Node : ConnectedNodes)
	{
		// Root is always in zero depth
		if (Node->IsA<UQuestGraphNode_Root>())
		{
			NodeDepths.Add(Node, 0);
		}
		else
		{
			TArray<UQuestGraphNodeCommonBase*> Path;
			TSet<UQuestGraphNodeCommonBase*> Visited;

			const bool bFound = SolvePathToRoot(Node, Path, Visited);
			if (!ensure(bFound && Path.Num() > 0))
			{
				continue;
			}

			NodeDepths.Add(Node, Path.Num());
		}
	}
}

TArray<UQuestGraphNodeCommonBase*> FQuestCompiler::GetNodesInDepth(int32 Depth) const
{
	TArray<UQuestGraphNodeCommonBase*> Output;
	for (const TPair<UQuestGraphNodeCommonBase*, int32>& DepthKV : NodeDepths)
	{
		if (DepthKV.Value == Depth)
		{
			Output.Add(DepthKV.Key);
		}
	}
	return Output;
}

TArray<UQuestGraphNodeCommonBase*> FQuestCompiler::SortChildren(const TArray<UQuestGraphNodeCommonBase*>& Children) const
{
	TArray<UQuestGraphNodeCommonBase*> SortedChildren;
	SortedChildren.Append(Children);

	SortedChildren.Sort([](const UQuestGraphNodeCommonBase& A, const UQuestGraphNodeCommonBase& B)
		{
			return A.GetPosition().X < B.GetPosition().X;
		});

	return SortedChildren;
}

TArray<UQuestGraphNodeCommonBase*> FQuestCompiler::GetAndSortNodeChildren(UQuestGraphNodeCommonBase* Node) const
{
	check(Node);
	return SortChildren(Node->GetChildrenNodes());
}

void FQuestCompiler::SolveNodeIndicies()
{
	// Set Node indicies of all node to none by default
	UQuestGraph* Graph = CastChecked<UQuestGraph>(QuestAsset->GetGraph());
	TArray<UEdGraphNode*> AllNodes = Graph->GetGraphNodes();
	for (UEdGraphNode* BaseNode : AllNodes)
	{
		if (UQuestGraphNodeCommonBase* QuestNode = Cast<UQuestGraphNodeCommonBase>(BaseNode))
		{
			QuestNode->SetQuestNodeIndex(INDEX_NONE);
		}
	}

	// Find last depth
	int32 MaxIterDepth = 0;
	for (const TPair<UQuestGraphNodeCommonBase*, int32>& DepthKV : NodeDepths)
	{
		if (DepthKV.Value > MaxIterDepth)
		{
			MaxIterDepth = DepthKV.Value;
		}
	}

	// Start again from zero
	NextNodeIndex = 0;
	IndexGivenNodes.Empty();

	// Find root and give it index 0
	TArray<UQuestGraphNodeCommonBase*> RootDepthNodes = GetNodesInDepth(0);
	if (!ensure(RootDepthNodes.Num() == 1))
	{
		return;
	}

	ensure(RootDepthNodes[0]->IsA<UQuestGraphNode_Root>());
	RootDepthNodes[0]->SetQuestNodeIndex(GetNextNodeIndex());
	ensure(RootDepthNodes[0]->GetQuestNodeIndex() == 0);
	IndexGivenNodes.Add(RootDepthNodes[0]);

	// Iterate per depth
	for (int32 CurrentIterDepth = 0; CurrentIterDepth < MaxIterDepth; CurrentIterDepth++)
	{
		TArray<UQuestGraphNodeCommonBase*> DepthNodes = GetNodesInDepth(CurrentIterDepth);
		DepthNodes = SortChildren(DepthNodes); // Sort by X order

		// Iterate all nodes and sort their children to assign indicies
		for (UQuestGraphNodeCommonBase* Node : DepthNodes)
		{
			TArray<UQuestGraphNodeCommonBase*> SortedChildren = GetAndSortNodeChildren(Node);
			for (int32 IdxChild = 0; IdxChild < SortedChildren.Num(); IdxChild++)
			{
				UQuestGraphNodeCommonBase* Child = SortedChildren[IdxChild];
				check(Child);
				if (!IndexGivenNodes.Contains(Child))
				{
					const int32 Index = GetNextNodeIndex();
					Child->SetQuestNodeIndex(Index);
					IndexGivenNodes.Add(Child);
				}
			}
		}
	}
}

void FQuestCompiler::CompileNodes()
{
	check(QuestAsset);

	if (!ValidateConnectedGraphNodes())
	{
		return;
	}

	// Map indicies of graph nodes
	int32 MaxIndex = -1;
	TMap<int32, UQuestGraphNodeCommonBase*> IndexToNodeTable;
	TMap<UQuestGraphNodeCommonBase*, int32> NodeToIndexTable;
	for (UQuestGraphNodeCommonBase* GraphNode : ConnectedNodes)
	{
		if (!ensure(GraphNode->GetQuestNodeIndex() != INDEX_NONE))
		{
			continue;
		}

		if (MaxIndex < GraphNode->GetQuestNodeIndex())
		{
			MaxIndex = GraphNode->GetQuestNodeIndex();
		}

		ensure(!IndexToNodeTable.Contains(GraphNode->GetQuestNodeIndex()));
		ensure(!NodeToIndexTable.Contains(GraphNode));
		IndexToNodeTable.Add(GraphNode->GetQuestNodeIndex(), GraphNode);
		NodeToIndexTable.Add(GraphNode, GraphNode->GetQuestNodeIndex());
	}

	// Create result node array
	ResultNodes.Empty();
	for (int32 Index = 0; Index <= MaxIndex; Index++)
	{
		check(IndexToNodeTable.Contains(Index));
		UQuestGraphNodeCommonBase* GraphNode = IndexToNodeTable[Index];
		check(GraphNode);

		// Construct new object as copy of quest node in graph node
		UQuestNode* GraphNodeStoredNode = GraphNode->GetQuestNodeChecked<UQuestNode>();
		UQuestNode* CompileNode = NewObject<UQuestNode>(QuestAsset, GraphNodeStoredNode->GetClass(), NAME_None, RF_NoFlags, GraphNodeStoredNode);
		check(CompileNode);

		CompileNode->SetNodeIndex(Index);
		CompileNode->SetNodeGuid(GraphNodeStoredNode->GetNodeGuid());

		ResultNodes.Add(CompileNode);
	}
	ensure(ResultNodes.Num() - 1 == MaxIndex);

	// Assign children for the nodes
	for (UQuestGraphNodeCommonBase* GraphNode : ConnectedNodes)
	{
		check(NodeToIndexTable.Contains(GraphNode));
		const int32 CurrentNodeIndex = NodeToIndexTable[GraphNode];

		if (UQuestGraphNode_Stage* StageGraphNode = Cast<UQuestGraphNode_Stage>(GraphNode))
		{
			// Stage node can have only 2 children (completed or failed)
			TArray<UQuestGraphNodeCommonBase*> CompleteChildNodes = StageGraphNode->GetChildrenNodesOfPin(true);
			TArray<UQuestGraphNodeCommonBase*> FailedChildNodes = StageGraphNode->GetChildrenNodesOfPin(false);

			int32 ChildIndexCompleted = QUEST_INDEX_NONE;
			int32 ChildIndexFailed = QUEST_INDEX_NONE;

			if (CompleteChildNodes.Num() > 0)
			{
				check(NodeToIndexTable.Contains(CompleteChildNodes[0]));
				ChildIndexCompleted = NodeToIndexTable[CompleteChildNodes[0]];
			}

			if (FailedChildNodes.Num() > 0)
			{
				check(NodeToIndexTable.Contains(FailedChildNodes[0]));
				ChildIndexFailed = NodeToIndexTable[FailedChildNodes[0]];
			}

			TArray<int32> ChildIndexArray;
			ChildIndexArray.Add(ChildIndexCompleted);
			ChildIndexArray.Add(ChildIndexFailed);

			check(ChildIndexArray.Num() == 2);

			check(ResultNodes.IsValidIndex(CurrentNodeIndex));
			ResultNodes[CurrentNodeIndex]->SetChildrenIndicies(ChildIndexArray);
		}
		else // Other nodes sort their children by node order
		{
			TArray<UQuestGraphNodeCommonBase*> ChildNodes = GraphNode->GetChildrenNodes();
			ChildNodes.Sort([](const UQuestGraphNodeCommonBase& A, const UQuestGraphNodeCommonBase& B)
				{
					return A.GetQuestNodeIndex() < B.GetQuestNodeIndex();
				});

			// Get index of children
			TArray<int32> ChildIndexArray;
			for (UQuestGraphNodeCommonBase* Child : ChildNodes)
			{
				check(NodeToIndexTable.Contains(Child));
				const int32 ChildIndex = NodeToIndexTable[Child];
				ChildIndexArray.Add(ChildIndex);
			}

			check(ResultNodes.IsValidIndex(CurrentNodeIndex));
			ResultNodes[CurrentNodeIndex]->SetChildrenIndicies(ChildIndexArray);

			// If this is selector node
			if (UQuestGraphNode_Selector* SelectorGraphNode = Cast<UQuestGraphNode_Selector>(GraphNode))
			{
				UQuestNode_Selector* Selector = Cast<UQuestNode_Selector>(ResultNodes[CurrentNodeIndex]);
				if (ensure(Selector))
				{
					CompileSelector(SelectorGraphNode, Selector, IndexToNodeTable);
				}
			}
		}
	}

	// Validate first and if everything okay, set results to quest
	if (ValidateCompilation())
	{
		// Set nodes to the quest
		QuestAsset->SetQuestNodes(ResultNodes);
	}
}

void FQuestCompiler::CompileSelector(UQuestGraphNode_Selector* GraphNode, UQuestNode_Selector* Selector, const TMap<int32, UQuestGraphNodeCommonBase*>& NodeTable)
{
	check(GraphNode && Selector);

	TMap<int32, int32> ConditionIndexTable;
	TArray<FQuestConditions> Conditions;

	TArray<UQuestGraphNode_Connection*> Connections = GraphNode->GetConnectionsToChildren();
	const TArray<int32>& SelectorChildren = Selector->GetChildren();

	for (int32 iChildIndex = 0; iChildIndex < SelectorChildren.Num(); iChildIndex++)
	{
		// Get node index of child and search the node
		const int32 ChildNodeIndex = SelectorChildren[iChildIndex];
		if (!ensure(NodeTable.Contains(ChildNodeIndex)))
		{
			continue;
		}
		const UQuestGraphNodeCommonBase* ChildNode = NodeTable[ChildNodeIndex];

		// Find connection that leads to child node
		bool bFoundChildConnection = false;
		for (UQuestGraphNode_Connection* Connection : Connections)
		{
			check(Connection);
			if (ChildNode == Connection->GetChildNode())
			{
				// Add condition instance to selector node
				const FQuestConditions& ConnectionConditions = Connection->GetConditions();

				FQuestConditions ResultCondition = FQuestConditions::MakeNewInstance(Selector, ConnectionConditions);
				const int32 AddedIndex = Conditions.Add(ResultCondition);

				ensure(!ConditionIndexTable.Contains(iChildIndex));
				ConditionIndexTable.Add(iChildIndex, AddedIndex);

				bFoundChildConnection = true;
				break;
			}
		}
		ensure(bFoundChildConnection);
	}

	Selector->SetConditions(Conditions, ConditionIndexTable);
}

void FQuestCompiler::ClearNodeErrors()
{
	UQuestGraph* Graph = CastChecked<UQuestGraph>(QuestAsset->GetGraph());
	const TArray<UEdGraphNode*>& AllNodes = Graph->GetGraphNodes();
	for (UEdGraphNode* Node : AllNodes)
	{
		if (UQuestGraphNodeCommonBase* CommonNode = Cast<UQuestGraphNodeCommonBase>(Node))
		{
			CommonNode->ClearCompilerMessage();
		}
	}
}

bool FQuestCompiler::ValidateConnectedGraphNodes()
{
	bool bHadError = false;

	// Check stage name uniqueness
	TSet<FName> StageNames;
	for (UQuestGraphNodeCommonBase* CommonNode : ConnectedNodes)
	{
		if (UQuestGraphNode_Stage* StageNode = Cast<UQuestGraphNode_Stage>(CommonNode))
		{
			UQuestNode_Stage* Stage = StageNode->GetQuestNodeChecked<UQuestNode_Stage>();
			if (StageNames.Contains(Stage->GetLogicalName()))
			{
				StageNode->SetCompilerErrorMessage(FString::Printf(TEXT("Stages must have unique names! %s is already used in another stage."), *Stage->GetLogicalName().ToString()));
				bHadError = true;
			}
			StageNames.Add(Stage->GetLogicalName());
		}
	}

	// Check objective uniqueness
	for (UQuestGraphNodeCommonBase* CommonNode : ConnectedNodes)
	{
		if (UQuestGraphNode_Stage* StageNode = Cast<UQuestGraphNode_Stage>(CommonNode))
		{
			UQuestNode_Stage* Stage = StageNode->GetQuestNodeChecked<UQuestNode_Stage>();
			TArray<UQuestObjective*> Objectives = Stage->GetObjectives();

			TSet<FName> ObjectiveNames;
			for (UQuestObjective* Objective : Objectives)
			{
				if (!Objective)
				{
					StageNode->SetCompilerErrorMessage(FString::Printf(TEXT("Invalid objective in stage %s"), *Stage->GetLogicalName().ToString()));
					bHadError = true;
					continue;
				}

				if (ObjectiveNames.Contains(Objective->LogicalName))
				{
					StageNode->SetCompilerErrorMessage(FString::Printf(TEXT("Objectives must have unique names in stage! %s is already name of an objective in stage %s."), *Objective->LogicalName.ToString(), *Stage->GetLogicalName().ToString()));
					bHadError = true;
				}
				ObjectiveNames.Add(Objective->LogicalName);
			}
		}
	}

	return !bHadError;
}

bool FQuestCompiler::ValidateCompilation()
{
	// TODO
	return true;
}