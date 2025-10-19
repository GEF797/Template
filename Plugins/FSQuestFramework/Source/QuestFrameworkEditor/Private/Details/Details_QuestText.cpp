// Copyright Sami Kangasmaa, 2022


#include "Details/Details_QuestText.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "STextPropertyEditableTextBox.h"

#define LOCTEXT_NAMESPACE "Details_QuestText"

namespace QuestTextPropertyCustomization
{
	/** Allows STextPropertyEditableTextBox to edit a property handle */
	class FEditableTextPropertyHandle : public IEditableTextProperty
	{
	public:
		FEditableTextPropertyHandle(const TSharedRef<IPropertyHandle>& InPropertyHandle, const TSharedPtr<IPropertyUtilities>& InPropertyUtilities)
			: PropertyHandle(InPropertyHandle)
			, PropertyUtilities(InPropertyUtilities)
		{
		}

		virtual bool IsMultiLineText() const override
		{
			return PropertyHandle->IsValidHandle() && PropertyHandle->GetBoolMetaData("MultiLine");
		}

		virtual bool IsPassword() const override
		{
			return PropertyHandle->IsValidHandle() && PropertyHandle->GetBoolMetaData("PasswordField");
		}

		virtual bool IsReadOnly() const override
		{
			return !PropertyHandle->IsValidHandle() || PropertyHandle->IsEditConst();
		}

		virtual bool IsDefaultValue() const override
		{
			return PropertyHandle->IsValidHandle() && !PropertyHandle->DiffersFromDefault();
		}

		virtual FText GetToolTipText() const override
		{
			return (PropertyHandle->IsValidHandle())
				? PropertyHandle->GetToolTipText()
				: FText::GetEmpty();
		}

		virtual int32 GetNumTexts() const override
		{
			return (PropertyHandle->IsValidHandle())
				? PropertyHandle->GetNumPerObjectValues()
				: 0;
		}

		virtual FText GetText(const int32 InIndex) const override
		{
			if (PropertyHandle->IsValidHandle())
			{
				FString ObjectValue;
				if (PropertyHandle->GetPerObjectValue(InIndex, ObjectValue) == FPropertyAccess::Success)
				{
					FText TextValue;
					if (FTextStringHelper::ReadFromBuffer(*ObjectValue, TextValue))
					{
						return TextValue;
					}
				}
			}

			return FText::GetEmpty();
		}

		virtual void SetText(const int32 InIndex, const FText& InText) override
		{
			if (PropertyHandle->IsValidHandle())
			{
				FString ObjectValue;
				FTextStringHelper::WriteToBuffer(ObjectValue, InText);
				PropertyHandle->SetPerObjectValue(InIndex, ObjectValue);
			}
		}

		virtual bool IsValidText(const FText& InText, FText& OutErrorMsg) const override
		{
			return true;
		}

#if USE_STABLE_LOCALIZATION_KEYS
		virtual void GetStableTextId(const int32 InIndex, const ETextPropertyEditAction InEditAction, const FString& InTextSource, const FString& InProposedNamespace, const FString& InProposedKey, FString& OutStableNamespace, FString& OutStableKey) const override
		{
			if (PropertyHandle->IsValidHandle())
			{
				TArray<UPackage*> PropertyPackages;
				PropertyHandle->GetOuterPackages(PropertyPackages);

				check(PropertyPackages.IsValidIndex(InIndex));

				StaticStableTextId(PropertyPackages[InIndex], InEditAction, InTextSource, InProposedNamespace, InProposedKey, OutStableNamespace, OutStableKey);
			}
		}
#endif // USE_STABLE_LOCALIZATION_KEYS

	private:
		TSharedRef<IPropertyHandle> PropertyHandle;
		TSharedPtr<IPropertyUtilities> PropertyUtilities;
	};
}

void FDetails_QuestText::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TSharedPtr<IPropertyHandle> TextProperty = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestText, Text));

	TSharedRef<IEditableTextProperty> EditableTextProperty = MakeShareable(new QuestTextPropertyCustomization::FEditableTextPropertyHandle(TextProperty.ToSharedRef(), StructCustomizationUtils.GetPropertyUtilities()));
	const bool bIsMultiLine = EditableTextProperty->IsMultiLineText();

	HeaderRow.NameContent()
		[
			StructPropertyHandle->CreatePropertyNameWidget()
		];

	HeaderRow.ValueContent()
		.MinDesiredWidth(bIsMultiLine ? 250.f : 125.f)
		.MaxDesiredWidth(600.f)
		[
			SNew(STextPropertyEditableTextBox, EditableTextProperty)
			.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
			.AutoWrapText(true)
		];
}

void FDetails_QuestText::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TSharedPtr<IPropertyHandle> ArgsProperty = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestText, TextArguments));

	TSharedRef<FDetailArrayBuilder> ArgArrayBuilder = MakeShareable(new FDetailArrayBuilder(ArgsProperty.ToSharedRef(), false, false, false));
	ArgArrayBuilder->OnGenerateArrayElementWidget(FOnGenerateArrayElementWidget::CreateSP(this, &FDetails_QuestText::GenerateArgArrayElementWidget));

	StructBuilder.AddCustomBuilder(ArgArrayBuilder);
}

void FDetails_QuestText::GenerateArgArrayElementWidget(TSharedRef<IPropertyHandle> PropertyHandle, int32 ArrayIndex, IDetailChildrenBuilder& ChildrenBuilder)
{
	ChildrenBuilder.AddProperty(PropertyHandle);
}

// ------------------------------------------------------------------------ //

void FDetails_QuestTextArg::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	PropertyUtilities = StructCustomizationUtils.GetPropertyUtilities();

	ArgTypePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestTextArg, Type));
	ArgTypePropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FDetails_QuestTextArg::OnArgTypeChanged, true));

	FormatTogglePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestTextArg, bUseNumberFormatter));
	FormatTogglePropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FDetails_QuestTextArg::OnFormatToggleChanged, true));

	TSharedPtr<IPropertyHandle> ArgNameProperty = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestTextArg, ArgName));
	HeaderRow.NameContent()
		[
			ArgNameProperty->CreatePropertyValueWidget()
		];
}

void FDetails_QuestTextArg::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	StructBuilder.AddProperty(ArgTypePropertyHandle.ToSharedRef());

	{
		IDetailPropertyRow& VarNamePropertyRow = StructBuilder.AddProperty(StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestTextArg, Variable)).ToSharedRef());
		VarNamePropertyRow.Visibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &FDetails_QuestTextArg::GetVariableNameVisibility)));
	}

	{
		IDetailPropertyRow& FormatTogglePropertyRow = StructBuilder.AddProperty(StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestTextArg, bUseNumberFormatter)).ToSharedRef());
		FormatTogglePropertyRow.Visibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &FDetails_QuestTextArg::GetNumberFormatToggleVisibility)));
	}

	{
		IDetailPropertyRow& FormatPropertyRow = StructBuilder.AddProperty(StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestTextArg, NumberFormatOptions)).ToSharedRef());
		FormatPropertyRow.Visibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &FDetails_QuestTextArg::GetNumberFormatVisibility)));
	}

	{
		IDetailPropertyRow& CustomArgPropertyRow = StructBuilder.AddProperty(StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FQuestTextArg, CustomTextArgument)).ToSharedRef());
		CustomArgPropertyRow.Visibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &FDetails_QuestTextArg::GetCustomArgVisibility)));
	}

	OnArgTypeChanged(false);
	OnFormatToggleChanged(false);
}

void FDetails_QuestTextArg::OnArgTypeChanged(bool bRefresh)
{
	uint8 Value = 0;
	if (ArgTypePropertyHandle->GetValue(Value) != FPropertyAccess::Success)
	{
		return;
	}
	ArgType = static_cast<EQuestTextArgType>(Value);

	if (bRefresh && PropertyUtilities.IsValid())
	{
		PropertyUtilities->ForceRefresh();
	}
}

void FDetails_QuestTextArg::OnFormatToggleChanged(bool bRefresh)
{
	bool Value = 0;
	if (FormatTogglePropertyHandle->GetValue(Value) != FPropertyAccess::Success)
	{
		return;
	}

	bUseFormatter = Value;

	if (bRefresh && PropertyUtilities.IsValid())
	{
		PropertyUtilities->ForceRefresh();
	}
}

#undef LOCTEXT_NAMESPACE