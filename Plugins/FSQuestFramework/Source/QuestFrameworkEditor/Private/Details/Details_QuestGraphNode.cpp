// Copyright Sami Kangasmaa, 2022


#include "Details/Details_QuestGraphNode.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

#include "Quest.h"

#include "Nodes/QuestNode.h"
#include "Nodes/QuestGraphNodeCommonBase.h"

#include "Nodes/QuestGraphNode_Connection.h"
#include "Nodes/QuestGraphNode_Selector.h"
#include "Nodes/QuestGraphNode_Root.h"


#define LOCTEXT_NAMESPACE "Details_QuestGraphNode"

void FDetails_QuestGraphNode::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> CustomizedObjects;
	DetailBuilder.GetObjectsBeingCustomized(CustomizedObjects);
	if (CustomizedObjects.Num() != 1)
	{
		return;
	}

	if (!CustomizedObjects[0].IsValid())
	{
		return;
	}

	UQuestGraphNodeCommonBase* GraphNode = Cast<UQuestGraphNodeCommonBase>(CustomizedObjects[0].Get());
	if (!IsValid(GraphNode))
	{
		return;
	}

	const UQuestNode* QuestNode = GraphNode->GetQuestNodeChecked<UQuestNode>();

	// Hide all by default
	DetailBuilder.HideCategory(UQuestGraphNodeCommonBase::StaticClass()->GetFName());
	DetailBuilder.HideCategory(TEXT("Quest Node"));

	// Don't show details for root or selectors
	if (GraphNode->IsA<UQuestGraphNode_Selector>() || GraphNode->IsA<UQuestGraphNode_Root>())
	{
		return;
	}

	// Show all quest node properties
	IDetailCategoryBuilder& NodeCategory = DetailBuilder.EditCategory(TEXT("Node"), FText::GetEmpty(), ECategoryPriority::Important);
	NodeCategory.InitiallyCollapsed(false);
	const TSharedPtr<IPropertyHandle> PropertyQuestNode = DetailBuilder.GetProperty(UQuestGraphNodeCommonBase::GetMemberNameChecked_QuestNode(), UQuestGraphNodeCommonBase::StaticClass());
	for (FProperty* Property = QuestNode->GetClass()->PropertyLink; Property != nullptr; Property = Property->PropertyLinkNext)
	{
		const TSharedPtr<IPropertyHandle> ChildProperty = PropertyQuestNode->GetChildHandle(Property->GetFName());

		if (!ChildProperty.IsValid())
		{
			continue;
		}
		NodeCategory.AddProperty(ChildProperty);
	}
}

void FDetails_QuestGraphNode_Connection::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> CustomizedObjects;
	DetailBuilder.GetObjectsBeingCustomized(CustomizedObjects);
	if (CustomizedObjects.Num() != 1)
	{
		return;
	}

	if (!CustomizedObjects[0].IsValid())
	{
		return;
	}

	UQuestGraphNode_Connection* Connection = Cast<UQuestGraphNode_Connection>(CustomizedObjects[0].Get());
	if (!IsValid(Connection))
	{
		return;
	}

	DetailBuilder.HideCategory(TEXT("Connection"));

	// Show condition properties only if connection has selector as parent
	if (Connection->HasSelectorParent())
	{
		IDetailCategoryBuilder& NodeCategory = DetailBuilder.EditCategory(TEXT("Condition"), FText::GetEmpty(), ECategoryPriority::Important);
		NodeCategory.InitiallyCollapsed(false);

		for (FProperty* Property = Connection->GetClass()->PropertyLink; Property != nullptr; Property = Property->PropertyLinkNext)
		{
			if (!Property)
			{
				continue;
			}
			NodeCategory.AddProperty(DetailBuilder.GetProperty(Property->GetFName(), UQuestGraphNode_Connection::StaticClass()));
		}
	}
}

#undef LOCTEXT_NAMESPACE