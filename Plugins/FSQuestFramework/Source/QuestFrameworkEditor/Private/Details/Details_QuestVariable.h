// Copyright, Sami Kangasmaa 2022

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
#include "QuestMemory.h"
#include "Quest.h"

#include "Details/Slate/SQuestVariablePicker.h"

// ---------------------------------------------- //

/**
 * Custom layout for quest variable picker
 */
class FDetails_QuestVariable : public IPropertyTypeCustomization
{
public:

	typedef FDetails_QuestVariable Self;

public:

	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShared<Self>(); }

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:

	EQuestVariableType VariableType = EQuestVariableType::Bool;
	UQuest* QuestAsset = nullptr;

	TSharedPtr<IPropertyUtilities> PropertyUtilities;

	TSharedPtr<IPropertyHandle> VariableTypePropertyHandle;
	TSharedPtr<IPropertyHandle> VariableNamePropertyHandle;
	TSharedPtr<SQuestVariablePicker> VariablePickerWidget;

private:

	void OnVariableTypeChanged(bool bRefresh);

	TArray<FName> GetAvailableVariables() const;
	UQuest* FindQuestAsset(TSharedRef<IPropertyHandle> PropertyHandle) const;
};


// ---------------------------------------------- //

/**
 * Custom layout for quest variable picker
 */
class FDetails_QuestVariableComparison : public IPropertyTypeCustomization
{
public:

	typedef FDetails_QuestVariableComparison Self;

public:

	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShared<Self>(); }

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:

	EQuestVariableComparisonType ComparisonType = EQuestVariableComparisonType::Constant;
	EQuestVariableType VariableType = EQuestVariableType::Bool;

	TSharedPtr<IPropertyUtilities> PropertyUtilities;

	TSharedPtr<IPropertyHandle> VariablePropertyHandle;
	TSharedPtr<IPropertyHandle> ComparisonTypePropertyHandle;
	TSharedPtr<IPropertyHandle> VariableTypePropertyHandle;

private:

	void OnComparisonTypeChanged(bool bRefresh);
	void OnVariableTypeChanged(bool bRefresh);
};

// ---------------------------------------------- //

/**
 * Custom layout for quest variable initializer to display only appropriate type of field
 */
class FDetails_QuestVariableInitializer : public IPropertyTypeCustomization
{
public:

	typedef FDetails_QuestVariableInitializer Self;

public:

	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShared<Self>(); }

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:

	EQuestVariableType VariableType = EQuestVariableType::Bool;

	TSharedPtr<IPropertyUtilities> PropertyUtilities;

	TSharedPtr<IPropertyHandle> VariablePropertyHandle;
	TSharedPtr<IPropertyHandle> VariableTypePropertyHandle;

private:

	void OnVariableTypeChanged(bool bRefresh);
};

// ------------------------------------------------------------------ //

/**
 * Custom layout for quest variable initializer to display only appropriate type of field
 */
class FDetails_QuestVariableBounds : public IPropertyTypeCustomization
{
public:

	typedef FDetails_QuestVariableBounds Self;

public:

	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShared<Self>(); }

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:

	EQuestVariableType VariableType = EQuestVariableType::Bool;

	TSharedPtr<IPropertyUtilities> PropertyUtilities;

	TSharedPtr<IPropertyHandle> VariablePropertyHandle;
	TSharedPtr<IPropertyHandle> VariableTypePropertyHandle;

private:

	void OnVariableTypeChanged(bool bRefresh);
};