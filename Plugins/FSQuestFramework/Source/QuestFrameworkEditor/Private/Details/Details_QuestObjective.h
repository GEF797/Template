// Copyright, Sami Kangasmaa 2022

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

#include "IPropertyTypeCustomization.h"
#include "Layout/Visibility.h"

#include "IDetailPropertyRow.h"
#include "IDetailGroup.h"
#include "IPropertyUtilities.h"
#include "IDetailChildrenBuilder.h"
#include "DetailWidgetRow.h"

#include "QuestTextTypes.h"
#include "QuestMemory.h"
#include "Quest.h"
#include "Nodes/QuestNode.h"
#include "QuestObjects/QuestObjective.h"
#include "QuestObjects/QuestObjectiveObserver.h"

// ----------------------------------------------------- //

/**
 * Details for objective list in quest stage
 */
class FDetails_QuestStageObjectives : public IPropertyTypeCustomization
{
	typedef FDetails_QuestStageObjectives Self;

public:

	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShared<Self>(); }

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:

	TSharedPtr<IPropertyUtilities> PropertyUtilities;

	void GenerateObjectiveArrayElementWidget(TSharedRef<IPropertyHandle> PropertyHandle, int32 ArrayIndex, IDetailChildrenBuilder& ChildrenBuilder);

	void OnObjectiveLogicalNameChanged(bool bRefresh);

	void OnObjectiveCustomObserverChanged(bool bRefresh);
};


// ----------------------------------------------------- //