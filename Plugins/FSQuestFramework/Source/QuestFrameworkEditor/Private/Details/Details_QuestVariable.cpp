// Copyright, Sami Kangasmaa 2022


#include "Details/Details_QuestVariable.h"

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "PropertyCustomizationHelpers.h"

#include "Quest.h"
#include "Nodes/QuestGraphNodeBase.h"
#include "Nodes/QuestNode.h"
#include "QuestEditor/QuestEditorUtils.h"

#define LOCTEXT_NAMESPACE "Details_QuestVariable"

void FDetails_QuestVariable::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	QuestAsset = FindQuestAsset(StructPropertyHandle);

	PropertyUtilities = StructCustomizationUtils.GetPropertyUtilities();

	VariableTypePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestVariable, Type));
	VariableTypePropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FDetails_QuestVariable::OnVariableTypeChanged, true));

	VariableNamePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestVariable, Variable));
}

void FDetails_QuestVariable::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	FDetailWidgetRow& Row = StructBuilder.AddCustomRow(StructPropertyHandle->GetPropertyDisplayName());

	Row.NameContent()
		[
			StructPropertyHandle->CreatePropertyNameWidget()
		];


	VariablePickerWidget = SNew(SQuestVariablePicker).AvailableVariableNames(this, &Self::GetAvailableVariables);
	VariablePickerWidget->SetPropertyHandle(VariableNamePropertyHandle);

	Row.ValueContent()
		.MinDesiredWidth(209.f)
		.MaxDesiredWidth(600.f)
		[
			SNew(SHorizontalBox)

			+SHorizontalBox::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Fill)
			.AutoWidth()
			[
				VariableTypePropertyHandle->CreatePropertyValueWidget()
			]

			+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					VariablePickerWidget.ToSharedRef()
				]
			+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					StructPropertyHandle->CreateDefaultPropertyButtonWidgets()
				]
		];

	OnVariableTypeChanged(false);
}

void FDetails_QuestVariable::OnVariableTypeChanged(bool bRefresh)
{
	uint8 Value = 0;
	if (VariableTypePropertyHandle->GetValue(Value) != FPropertyAccess::Success)
	{
		return;
	}

	VariableType = static_cast<EQuestVariableType>(Value);

	if (bRefresh && PropertyUtilities.IsValid())
	{
		PropertyUtilities->ForceRefresh();
	}
}


TArray<FName> FDetails_QuestVariable::GetAvailableVariables() const
{
	TArray<FName> Names;
	if (QuestAsset)
	{
		Names = FQuestEditorUtils::FindQuestVariableNames(QuestAsset, VariableType);
		Names.Sort([](const FName& A, const FName& B)
			{
				return A.LexicalLess(B);
			});
	}
	return Names;
}

UQuest* FDetails_QuestVariable::FindQuestAsset(TSharedRef<IPropertyHandle> PropertyHandle) const
{
	TArray<UObject*> Outers;
	PropertyHandle->GetOuterObjects(Outers);

	// Search for nodes
	for (UObject* Object : Outers)
	{
		if (UQuest* QuestObject = Cast<UQuest>(Object))
		{
			return QuestObject;
		}

		if (UQuestGraphNodeBase* GraphNode = Cast<UQuestGraphNodeBase>(Object))
		{
			if (UQuest* Quest = GraphNode->GetQuest())
			{
				return Quest;
			}
		}

		if (UQuestNode* Node = Cast<UQuestNode>(Object))
		{
			if (UQuest* Quest = Node->GetTypedOuter<UQuest>())
			{
				return Quest;
			}
		}
	}

	// Search from property handle chain
	TSharedPtr<IPropertyHandle> RootHandle = PropertyHandle->GetParentHandle();
	while (RootHandle.IsValid() && RootHandle->GetParentHandle().IsValid())
	{
		RootHandle = RootHandle->GetParentHandle();
	}

	if (RootHandle.IsValid())
	{
		TArray<UObject*> OuterObjects;
		RootHandle->GetOuterObjects(OuterObjects);
		for (UObject* Object : OuterObjects)
		{
			if (UQuest* QuestObject = Cast<UQuest>(Object))
			{
				return QuestObject;
			}
			else if (UQuest* OuterQuestObject = Object->GetTypedOuter<UQuest>())
			{
				return OuterQuestObject;
			}
		}
	}

	return nullptr;
}

// ------------------------------------------------------------------------------------- //

void FDetails_QuestVariableComparison::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	PropertyUtilities = StructCustomizationUtils.GetPropertyUtilities();

	ComparisonTypePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestVariableComparison, ComparisonType), true);
	if (ComparisonTypePropertyHandle.IsValid())
	{
		ComparisonTypePropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FDetails_QuestVariableComparison::OnComparisonTypeChanged, true));
	}

	VariablePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestVariableComparison, Variable), true);
	if (VariablePropertyHandle.IsValid())
	{
		VariableTypePropertyHandle = VariablePropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestVariable, Type), true);
		if (VariableTypePropertyHandle.IsValid())
		{
			VariablePropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FDetails_QuestVariableComparison::OnVariableTypeChanged, true));
		}
	}

	OnComparisonTypeChanged(false);
	OnVariableTypeChanged(false);
}

void FDetails_QuestVariableComparison::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	if (ComparisonTypePropertyHandle.IsValid())
	{
		StructBuilder.AddProperty(ComparisonTypePropertyHandle.ToSharedRef());
	}

	if (VariablePropertyHandle.IsValid())
	{
		StructBuilder.AddProperty(VariablePropertyHandle.ToSharedRef());
	}

	TSharedPtr<IPropertyHandle> OperatorProperty = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestVariableComparison, Operator), true);
	if (OperatorProperty.IsValid())
	{
		StructBuilder.AddProperty(OperatorProperty.ToSharedRef());
	}

	if (ComparisonType == EQuestVariableComparisonType::Constant)
	{
		TSharedPtr<IPropertyHandle> ConstProperty = nullptr;
		if (VariableType == EQuestVariableType::Bool)
		{
			ConstProperty = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestVariableComparison, bOtherBool), true);
		}
		else if (VariableType == EQuestVariableType::Float)
		{
			ConstProperty = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestVariableComparison, OtherFloat), true);
		}
		else if (VariableType == EQuestVariableType::Integer)
		{
			ConstProperty = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestVariableComparison, OtherInteger), true);
		}

		if (ConstProperty.IsValid())
		{
			StructBuilder.AddProperty(ConstProperty.ToSharedRef());
		}
	}
	else // Other variable
	{
		TSharedPtr<IPropertyHandle> OtherVariableProperty = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestVariableComparison, OtherVariable), true);
		if (OtherVariableProperty.IsValid())
		{
			StructBuilder.AddProperty(OtherVariableProperty.ToSharedRef());
		}
	}
}

void FDetails_QuestVariableComparison::OnComparisonTypeChanged(bool bRefresh)
{
	uint8 Value = 0;
	if (ComparisonTypePropertyHandle->GetValue(Value) != FPropertyAccess::Success)
	{
		return;
	}

	ComparisonType = static_cast<EQuestVariableComparisonType>(Value);

	if (bRefresh && PropertyUtilities.IsValid())
	{
		PropertyUtilities->ForceRefresh();
	}
}

void FDetails_QuestVariableComparison::OnVariableTypeChanged(bool bRefresh)
{
	uint8 Value = 0;
	if (VariableTypePropertyHandle->GetValue(Value) != FPropertyAccess::Success)
	{
		return;
	}

	VariableType = static_cast<EQuestVariableType>(Value);

	if (bRefresh && PropertyUtilities.IsValid())
	{
		PropertyUtilities->ForceRefresh();
	}
}

// ------------------------------------------------------------------------------------- //

void FDetails_QuestVariableInitializer::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	PropertyUtilities = StructCustomizationUtils.GetPropertyUtilities();

	VariablePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestVariableInitializer, Variable), true);
	if (VariablePropertyHandle.IsValid())
	{
		VariableTypePropertyHandle = VariablePropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestVariable, Type), true);
		if (VariableTypePropertyHandle.IsValid())
		{
			VariablePropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FDetails_QuestVariableInitializer::OnVariableTypeChanged, true));
		}
	}

	OnVariableTypeChanged(false);
}

void FDetails_QuestVariableInitializer::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	if (VariablePropertyHandle.IsValid())
	{
		StructBuilder.AddProperty(VariablePropertyHandle.ToSharedRef());
	}

	if (VariableType == EQuestVariableType::Bool)
	{
		TSharedPtr<IPropertyHandle> ValueField = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestVariableInitializer, BoolValue), true);
		StructBuilder.AddProperty(ValueField.ToSharedRef());
	}
	else if (VariableType == EQuestVariableType::Float)
	{
		TSharedPtr<IPropertyHandle> ValueField = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestVariableInitializer, FloatValue), true);
		StructBuilder.AddProperty(ValueField.ToSharedRef());
	}
	else if (VariableType == EQuestVariableType::Integer)
	{
		TSharedPtr<IPropertyHandle> ValueField = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestVariableInitializer, IntegerValue), true);
		StructBuilder.AddProperty(ValueField.ToSharedRef());
	}

	// Don't show additive property for default variables
	bool bDefaultVariable = false;
	TSharedPtr<IPropertyHandle> ParentHandlePtr = StructPropertyHandle->GetParentHandle();
	if (ParentHandlePtr.IsValid())
	{
		const FStructProperty* PropertyPtr = CastField<FStructProperty>(ParentHandlePtr.Get()->GetProperty());
		if (PropertyPtr && PropertyPtr->Struct == FDefaultQuestVariable::StaticStruct())
		{
			bDefaultVariable = true;
		}
	}

	// Show additive only for integer or float
	if (VariableType != EQuestVariableType::Bool && !bDefaultVariable)
	{
		TSharedPtr<IPropertyHandle> AdditiveHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestVariableInitializer, bAdditive), true);
		StructBuilder.AddProperty(AdditiveHandle.ToSharedRef());
	}
}


void FDetails_QuestVariableInitializer::OnVariableTypeChanged(bool bRefresh)
{
	uint8 Value = 0;
	if (VariableTypePropertyHandle->GetValue(Value) != FPropertyAccess::Success)
	{
		return;
	}

	VariableType = static_cast<EQuestVariableType>(Value);

	if (bRefresh && PropertyUtilities.IsValid())
	{
		PropertyUtilities->ForceRefresh();
	}
}

// ------------------------------------------------------------------------------------- //

void FDetails_QuestVariableBounds::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	PropertyUtilities = StructCustomizationUtils.GetPropertyUtilities();

	VariablePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestVariableBounds, Variable), true);
	if (VariablePropertyHandle.IsValid())
	{
		VariableTypePropertyHandle = VariablePropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestVariable, Type), true);
		if (VariableTypePropertyHandle.IsValid())
		{
			VariablePropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FDetails_QuestVariableBounds::OnVariableTypeChanged, true));
		}
	}

	HeaderRow.NameContent()
		[
			StructPropertyHandle->CreatePropertyNameWidget()
		];

	OnVariableTypeChanged(false);
}

void FDetails_QuestVariableBounds::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	if (VariablePropertyHandle.IsValid())
	{
		StructBuilder.AddProperty(VariablePropertyHandle.ToSharedRef());
	}

	TSharedPtr<IPropertyHandle> OperationHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestVariableBounds, ClampOperation), true);
	StructBuilder.AddProperty(OperationHandle.ToSharedRef());

	if (VariableType == EQuestVariableType::Bool)
	{
		// No bounds for booleans
	}
	else if (VariableType == EQuestVariableType::Float)
	{
		TSharedPtr<IPropertyHandle> ValueFieldMin = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestVariableBounds, MinFloat), true);
		StructBuilder.AddProperty(ValueFieldMin.ToSharedRef());

		TSharedPtr<IPropertyHandle> ValueFieldMax = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestVariableBounds, MaxFloat), true);
		StructBuilder.AddProperty(ValueFieldMax.ToSharedRef());
	}
	else if (VariableType == EQuestVariableType::Integer)
	{
		TSharedPtr<IPropertyHandle> ValueFieldMin = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestVariableBounds, MinInteger), true);
		StructBuilder.AddProperty(ValueFieldMin.ToSharedRef());

		TSharedPtr<IPropertyHandle> ValueFieldMax = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestVariableBounds, MaxInteger), true);
		StructBuilder.AddProperty(ValueFieldMax.ToSharedRef());
	}
}

void FDetails_QuestVariableBounds::OnVariableTypeChanged(bool bRefresh)
{
	uint8 Value = 0;
	if (VariableTypePropertyHandle->GetValue(Value) != FPropertyAccess::Success)
	{
		return;
	}
	VariableType = static_cast<EQuestVariableType>(Value);
	if (bRefresh && PropertyUtilities.IsValid())
	{
		PropertyUtilities->ForceRefresh();
	}
}

#undef LOCTEXT_NAMESPACE