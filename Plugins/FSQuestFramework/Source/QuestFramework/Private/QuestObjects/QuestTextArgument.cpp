// Copyright Sami Kangasmaa, 2022


#include "QuestObjects/QuestTextArgument.h"
#include "QuestContext.h"

FText UQuestTextArgument::GetTextArgumentValue(const UQuestContext* Context) const
{
	if (!ensure(Context))
	{
		return FText::GetEmpty();
	}

	FQuestObjectWorldScope WorldScope(this, Context->GetWorld());
	return GetText(const_cast<UQuestContext*>(Context));
}