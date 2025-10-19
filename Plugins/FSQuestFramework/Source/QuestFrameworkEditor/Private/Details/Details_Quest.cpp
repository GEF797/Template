// Copyright Sami Kangasmaa, 2022


#include "Details/Details_Quest.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

#include "Quest.h"

#define LOCTEXT_NAMESPACE "Details_Quest"

void FDetails_Quest::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> CustomizedObjects;
	DetailBuilder.GetObjectsBeingCustomized(CustomizedObjects);
	if (CustomizedObjects.Num() != 1)
	{
		return;
	}

	IDetailCategoryBuilder& DataCategory = DetailBuilder.EditCategory(TEXT("Quest"));
	DataCategory.InitiallyCollapsed(false);
}

#undef LOCTEXT_NAMESPACE
