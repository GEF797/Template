// Copyright Sami Kangasmaa, 2022


#include "Factory/QuestFactory.h"
#include "Quest.h"

#include "ClassViewerFilter.h"
#include "Misc/MessageDialog.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/SClassPickerDialog.h"

#include "QuestObjects/QuestCondition.h"
#include "QuestObjects/QuestEvent.h"
#include "QuestObjects/QuestObjectiveObserver.h"
#include "QuestObjects/QuestService.h"
#include "QuestObjects/QuestTextArgument.h"
#include "QuestObjects/QuestCustomData.h"

#define LOCTEXT_NAMESPACE "QuestFramework"

UQuestFactory::UQuestFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UQuest::StaticClass();
}

UObject* UQuestFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UQuest* Quest = NewObject<UQuest>(InParent, Class, Name, Flags | RF_Transactional);
	return Quest;
}

// ------------------------------------------------------------------- //

namespace QuestObjectFactoryUtils
{
	class FQuestObjectClassViewerFilter : public IClassViewerFilter
	{
		EClassFlags IgnoreFlags = CLASS_Deprecated | CLASS_Hidden;

	public:

		TSet<const UClass*> AllowedClasses;

		virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
		{
			return !InClass->HasAnyClassFlags(IgnoreFlags) && InFilterFuncs->IfInChildOfClassesSet(AllowedClasses, InClass) != EFilterReturn::Failed && FKismetEditorUtilities::CanCreateBlueprintOfClass(InClass);
		}

		virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef<const IUnloadedBlueprintData> InUnloadedClassData, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
		{
			return !InUnloadedClassData->HasAnyClassFlags(IgnoreFlags) && InFilterFuncs->IfInChildOfClassesSet(AllowedClasses, InUnloadedClassData) != EFilterReturn::Failed;
		}
	};

	bool ShowClassPickerDialog(const FText& Title, UClass*& OutClass, UClass* Class)
	{
		TSharedPtr<FQuestObjectClassViewerFilter> Filter = MakeShareable(new FQuestObjectClassViewerFilter);
		Filter->AllowedClasses.Add(Class);

		FClassViewerInitializationOptions Options;
		Options.Mode = EClassViewerMode::ClassPicker;
		Options.DisplayMode = EClassViewerDisplayMode::DefaultView;
		Options.ClassFilters.Add(Filter.ToSharedRef());
		Options.bExpandRootNodes = true;
		Options.bShowUnloadedBlueprints = true;
		Options.NameTypeToDisplay = EClassViewerNameTypeToDisplay::Dynamic;
		return SClassPickerDialog::PickClass(Title, Options, OutClass, Class);
	}
}

// ----------------------------------------------------------------- //

UQuestObjectFactoryBase::UQuestObjectFactoryBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = false;
	SupportedClass = UQuestObjectBase::StaticClass();
	ParentClass = SupportedClass;
	Title = FText::FromString(TEXT("Pick Parent Class for Quest Object"));
}

bool UQuestObjectFactoryBase::ConfigureProperties()
{
	ParentClass = nullptr;
	UClass* PickedClass = nullptr;
	const bool bSuccess = QuestObjectFactoryUtils::ShowClassPickerDialog(Title, PickedClass, SupportedClass);
	if (bSuccess)
	{
		ParentClass = PickedClass;
	}

	return bSuccess;
}

UObject* UQuestObjectFactoryBase::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name,EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	if (!ParentClass || !FKismetEditorUtilities::CanCreateBlueprintOfClass(ParentClass))
	{
		FFormatOrderedArguments Args;
		Args.Add(ParentClass ? FText::FromString(ParentClass->GetName()) : LOCTEXT("InvalidParentClassKey", "(invalid)"));
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("FailedToCreateChildBlueprintKey", "Failed to create a child blueprint from class '{0}'."), Args));
		return nullptr;
	}
	return FKismetEditorUtilities::CreateBlueprint(ParentClass, InParent, Name, BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass(), NAME_None);
}

// ----------------------------------------------------------------- //

UQuestEventFactory::UQuestEventFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UQuestEvent::StaticClass();
	ParentClass = SupportedClass;
	Title = FText::FromString(TEXT("Pick Parent Class for Quest Event"));
}

UQuestConditionFactory::UQuestConditionFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UQuestCondition::StaticClass();
	ParentClass = SupportedClass;
	Title = FText::FromString(TEXT("Pick Parent Class for Quest Condition"));
}

UQuestObjectiveObserverFactory::UQuestObjectiveObserverFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UQuestObjectiveObserver::StaticClass();
	ParentClass = SupportedClass;
	Title = FText::FromString(TEXT("Pick Parent Class for Objective Observer"));
}

UQuestServiceFactory::UQuestServiceFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UQuestService_BlueprintBase::StaticClass();
	ParentClass = SupportedClass;
	Title = FText::FromString(TEXT("Pick Parent Class for Quest Service"));
}

UQuestTextArgumentFactory::UQuestTextArgumentFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UQuestTextArgument::StaticClass();
	ParentClass = SupportedClass;
	Title = FText::FromString(TEXT("Pick Parent Class for Quest Text Argument"));
}

UQuestCustomDataFactory::UQuestCustomDataFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UQuestCustomData::StaticClass();
	ParentClass = SupportedClass;
	Title = FText::FromString(TEXT("Pick Parent Class for Quest Custom Data"));
}

#undef LOCTEXT_NAMESPACE