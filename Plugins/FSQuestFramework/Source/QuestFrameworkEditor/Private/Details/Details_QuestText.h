// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"

#include "IDetailCustomization.h"
#include "IPropertyTypeCustomization.h"
#include "Layout/Visibility.h"

#include "IDetailPropertyRow.h"
#include "IDetailGroup.h"
#include "IPropertyUtilities.h"
#include "IDetailChildrenBuilder.h"
#include "UObject/TextProperty.h"
#include "DetailWidgetRow.h"

#include "QuestData.h"
#include "QuestTextTypes.h"
#include "QuestObjects/QuestTextArgument.h"

/**
 * Custom details for quest text entries with dynamic arguments
 */

class FDetails_QuestText : public IPropertyTypeCustomization
{
	typedef FDetails_QuestText Self;

public:

	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShared<Self>(); }

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	void GenerateArgArrayElementWidget(TSharedRef<IPropertyHandle> PropertyHandle, int32 ArrayIndex, IDetailChildrenBuilder& ChildrenBuilder);
};

class FDetails_QuestTextArg : public IPropertyTypeCustomization
{
	typedef FDetails_QuestTextArg Self;

public:

	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShared<Self>(); }

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:

	TSharedPtr<IPropertyUtilities> PropertyUtilities;
	TSharedPtr<IPropertyHandle> ArgTypePropertyHandle;
	TSharedPtr<IPropertyHandle> FormatTogglePropertyHandle;

	EQuestTextArgType ArgType = EQuestTextArgType::QuestVariable;
	bool bUseFormatter = false;

	void OnArgTypeChanged(bool bRefresh);
	void OnFormatToggleChanged(bool bRefresh);

	EVisibility GetVariableNameVisibility() const
	{
		return ArgType == EQuestTextArgType::Custom ? EVisibility::Hidden : EVisibility::Visible;
	}

	EVisibility GetCustomArgVisibility() const
	{
		return ArgType == EQuestTextArgType::Custom ? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetNumberFormatToggleVisibility() const
	{
		return ArgType == EQuestTextArgType::Custom ? EVisibility::Hidden : EVisibility::Visible;
	}

	EVisibility GetNumberFormatVisibility() const
	{
		return ArgType == EQuestTextArgType::Custom ? EVisibility::Hidden : (bUseFormatter ? EVisibility::Visible : EVisibility::Hidden);
	}
};