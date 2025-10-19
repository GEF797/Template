// Copyright, Sami Kangasmaa 2022


#include "Details/Details_QuestObjective.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "PropertyCustomizationHelpers.h"

#define LOCTEXT_NAMESPACE "Details_QuestObjective"

// ----------------------------------------------------- //

void FDetails_QuestStageObjectives::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	PropertyUtilities = StructCustomizationUtils.GetPropertyUtilities();

	TSharedPtr<IPropertyHandle> ArrayProperty = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestStageObjectives, ObjectiveArray));
	HeaderRow.NameContent()
		[
			StructPropertyHandle->CreatePropertyNameWidget()
		];

	HeaderRow.ValueContent()
		[
			ArrayProperty->CreatePropertyValueWidget()
		];
}

void FDetails_QuestStageObjectives::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TSharedPtr<IPropertyHandle> ArrayProperty = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestStageObjectives, ObjectiveArray));

	TSharedRef<FDetailArrayBuilder> ObjectiveArrayBuilder = MakeShareable(new FDetailArrayBuilder(ArrayProperty.ToSharedRef(), false, false, false));
	ObjectiveArrayBuilder->OnGenerateArrayElementWidget(FOnGenerateArrayElementWidget::CreateSP(this, &FDetails_QuestStageObjectives::GenerateObjectiveArrayElementWidget));

	StructBuilder.AddCustomBuilder(ObjectiveArrayBuilder);
}

void FDetails_QuestStageObjectives::GenerateObjectiveArrayElementWidget(TSharedRef<IPropertyHandle> PropertyHandle, int32 ArrayIndex, IDetailChildrenBuilder& ChildrenBuilder)
{
	UObject* ObjectivePropertyValue = nullptr;
	FPropertyAccess::Result Result = PropertyHandle->GetValue(ObjectivePropertyValue);
	if (Result != FPropertyAccess::Success || !IsValid(ObjectivePropertyValue))
	{
		return;
	}
	const UQuestObjective* Objective = CastChecked<UQuestObjective>(ObjectivePropertyValue);

	IDetailGroup& Group = ChildrenBuilder.AddGroup(Objective->LogicalName, FText::FromName(Objective->LogicalName));

	TSharedPtr<IPropertyHandle> LogicalNameProperty = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(UQuestObjective, LogicalName));
	if (!ensure(LogicalNameProperty.IsValid()))
	{
		return;
	}

	LogicalNameProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FDetails_QuestStageObjectives::OnObjectiveLogicalNameChanged, true));

	// Add entry as objective name and button to delete the entry
	FDetailWidgetRow& Row = Group.HeaderRow();
	Row.NameContent()
		.MinDesiredWidth(209.f)
		.MaxDesiredWidth(600.f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Fill)
			.AutoWidth()
			[
				LogicalNameProperty->CreatePropertyValueWidget()
			]

			+ SHorizontalBox::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Fill)
			.AutoWidth()
			[
				PropertyHandle->CreateDefaultPropertyButtonWidgets()
			]
		];

	// Objective properties
	TSharedPtr<IPropertyHandle> DescriptionProperty = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(UQuestObjective, ShortDescription));
	if (DescriptionProperty.IsValid())
	{
		Group.AddPropertyRow(DescriptionProperty.ToSharedRef());
	}

	TSharedPtr<IPropertyHandle> LongDescriptionProperty = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(UQuestObjective, LongDescription));
	if (LongDescriptionProperty.IsValid())
	{
		Group.AddPropertyRow(LongDescriptionProperty.ToSharedRef());
	}

	TSharedPtr<IPropertyHandle> SignificancyProperty = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(UQuestObjective, Significancy));
	if (SignificancyProperty.IsValid())
	{
		Group.AddPropertyRow(SignificancyProperty.ToSharedRef());
	}

	// Observer
	TSharedPtr<IPropertyHandle> ObserverProperty = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(UQuestObjective, Observer));
	if (ObserverProperty.IsValid())
	{
		const bool bCustomObserver = Objective->bUseCustomObserver;
		if (bCustomObserver)
		{
			Group.AddPropertyRow(ObserverProperty.ToSharedRef());
		}
		else
		{
			UObject* ObserverPropertyValue = nullptr;
			FPropertyAccess::Result ObserverPropertyResult = ObserverProperty->GetValue(ObserverPropertyValue);
			if (ObserverPropertyResult == FPropertyAccess::Success && IsValid(ObserverPropertyValue))
			{
				if (const UObjectiveObserver_VariableQuery* VariableObserver = Cast<UObjectiveObserver_VariableQuery>(ObserverPropertyValue))
				{
					TSharedPtr<IPropertyHandle> CompleteQueryProperty = ObserverProperty->GetChildHandle(GET_MEMBER_NAME_CHECKED(UObjectiveObserver_VariableQuery, CompleteQuery));
					TSharedPtr<IPropertyHandle> FailQueryProperty = ObserverProperty->GetChildHandle(GET_MEMBER_NAME_CHECKED(UObjectiveObserver_VariableQuery, FailQuery));

					if (CompleteQueryProperty.IsValid() && FailQueryProperty.IsValid())
					{
						Group.AddPropertyRow(CompleteQueryProperty.ToSharedRef());
						Group.AddPropertyRow(FailQueryProperty.ToSharedRef());
					}
				}	
			}
		}
	}
	// Advanced
	IDetailGroup& AdvancedGroup = Group.AddGroup(FName(Objective->LogicalName.ToString() + "_advanced"), LOCTEXT("AdvancedGroupKey", "Advanced"));

	TSharedPtr<IPropertyHandle> AutoActivateProperty =  PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(UQuestObjective, bAutoActivate));
	if (AutoActivateProperty.IsValid())
	{
		AdvancedGroup.AddPropertyRow(AutoActivateProperty.ToSharedRef());
	}

	TSharedPtr<IPropertyHandle> CustomObserverToggleProperty = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(UQuestObjective, bUseCustomObserver));
	if (CustomObserverToggleProperty.IsValid())
	{
		CustomObserverToggleProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FDetails_QuestStageObjectives::OnObjectiveCustomObserverChanged, true));
		AdvancedGroup.AddPropertyRow(CustomObserverToggleProperty.ToSharedRef());
	}
}

void FDetails_QuestStageObjectives::OnObjectiveLogicalNameChanged(bool bRefresh)
{
	if (bRefresh && PropertyUtilities.IsValid())
	{
		PropertyUtilities->ForceRefresh();
	}
}

void FDetails_QuestStageObjectives::OnObjectiveCustomObserverChanged(bool bRefresh)
{
	if (bRefresh && PropertyUtilities.IsValid())
	{
		PropertyUtilities->ForceRefresh();
	}
}

// ----------------------------------------------------- //

#undef LOCTEXT_NAMESPACE