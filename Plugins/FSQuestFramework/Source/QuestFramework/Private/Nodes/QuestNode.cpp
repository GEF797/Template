// Copyright Sami Kangasmaa, 2022


#include "Nodes/QuestNode.h"
#include "QuestContext.h"

void UQuestNode::PostLoad()
{
	Super::PostLoad();
	GenerateNodeGuid();
}

void UQuestNode::PostInitProperties()
{
	Super::PostInitProperties();

	if (HasAnyFlags(RF_ClassDefaultObject | RF_NeedLoad))
	{
		return;
	}
	GenerateNodeGuid();
}

void UQuestNode::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);
	GenerateNodeGuid();
}

void UQuestNode::PostEditImport()
{
	Super::PostEditImport();
	GenerateNodeGuid();
}

FText UQuestNode::GetNodeText(UQuestContext* Context) const
{
	return NodeText.GetFormattedText(Context);
}

void UQuestNode::GenerateNodeGuid()
{
	if (!NodeGuid.IsValid())
	{
		NodeGuid = FGuid::NewGuid();
	}
}

void UQuestNode::ExecuteEntryEvents(UQuestContext* Context, bool bWasStateRestored)
{
	for (UQuestEvent* QuestEvent : EntryEvents)
	{
		if (IsValid(QuestEvent))
		{
			if (!bWasStateRestored) // Trigger entry events as usual
			{
				QuestEvent->NotifyExecuteEvent(Context, FQuestEventPayload());
			}
			else
			{
				// Trigger only if event should trigger on state restore
				if (QuestEvent->ShouldTriggerOnStateRestore())
				{
					QuestEvent->NotifyExecuteEvent(Context, FQuestEventPayload());
				}
			}
		}
	}
}

#if WITH_EDITOR
void UQuestNode::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	FProperty* Property = PropertyChangedEvent.PropertyChain.GetActiveNode()->GetValue();
	FProperty* MemberProperty = PropertyChangedEvent.PropertyChain.GetActiveMemberNode()->GetValue();
	if (Property && MemberProperty && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		static FName NAME_NodeTextProperty = GET_MEMBER_NAME_CHECKED(UQuestNode, NodeText);
		if (MemberProperty->GetFName() == NAME_NodeTextProperty && Property->GetFName() == FQuestText::GetTextPropertyName())
		{
			NodeText.NotifyEditText();
		}
	}

	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}
#endif