// Copyright, Sami Kangasmaa 2022

#pragma once

#include "CoreMinimal.h"

#include "Blueprint/BlueprintSupport.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"

#include "AssetTypeActions_Base.h"

#include "QuestObjects/QuestCondition.h"
#include "QuestObjects/QuestEvent.h"
#include "QuestObjects/QuestObjectiveObserver.h"
#include "QuestObjects/QuestService.h"
#include "QuestObjects/QuestTextArgument.h"
#include "QuestObjects/QuestCustomData.h"

class IToolkitHost;

/**
 * base class to derive quest object related blueprint creation action
 */
class FAssetTypeActions_QuestObjectBase : public FAssetTypeActions_Base
{
protected:

	EAssetTypeCategories::Type AssetCategory;

public:

	FAssetTypeActions_QuestObjectBase(EAssetTypeCategories::Type InAssetCategory) : AssetCategory(InAssetCategory) {}

	virtual FColor GetTypeColor() const override { return FColor(63, 126, 255); }

	virtual bool HasActions(const TArray<UObject*>& InObjects) const override { return false; }

	virtual uint32 GetCategories() override { return AssetCategory; }

	virtual bool CanFilter() override { return true; }
};

class FAssetTypeActions_QuestCondition : public FAssetTypeActions_QuestObjectBase
{
public:

	FAssetTypeActions_QuestCondition(EAssetTypeCategories::Type InAssetCategory) : FAssetTypeActions_QuestObjectBase(InAssetCategory) {}

	virtual UClass* GetSupportedClass() const override { return UQuestCondition::StaticClass(); }
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_QuestCondition", "Quest Condition"); }
};

class FAssetTypeActions_QuestEvent : public FAssetTypeActions_QuestObjectBase
{
public:

	FAssetTypeActions_QuestEvent(EAssetTypeCategories::Type InAssetCategory) : FAssetTypeActions_QuestObjectBase(InAssetCategory) {}

	virtual UClass* GetSupportedClass() const override { return UQuestEvent::StaticClass(); }
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_QuestEvent", "Quest Event"); }
};

class FAssetTypeActions_QuestService : public FAssetTypeActions_QuestObjectBase
{
public:

	FAssetTypeActions_QuestService(EAssetTypeCategories::Type InAssetCategory) : FAssetTypeActions_QuestObjectBase(InAssetCategory) {}

	virtual UClass* GetSupportedClass() const override { return UQuestService_BlueprintBase::StaticClass(); }
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_QuestService", "Quest Service"); }
};

class FAssetTypeActions_QuestObjectiveObserver : public FAssetTypeActions_QuestObjectBase
{
public:

	FAssetTypeActions_QuestObjectiveObserver(EAssetTypeCategories::Type InAssetCategory) : FAssetTypeActions_QuestObjectBase(InAssetCategory) {}

	virtual UClass* GetSupportedClass() const override { return UQuestObjectiveObserver::StaticClass(); }
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_QuestObserver", "Quest Objective Observer"); }
};

class FAssetTypeActions_QuestTextArgument : public FAssetTypeActions_QuestObjectBase
{
public:

	FAssetTypeActions_QuestTextArgument(EAssetTypeCategories::Type InAssetCategory) : FAssetTypeActions_QuestObjectBase(InAssetCategory) {}

	virtual UClass* GetSupportedClass() const override { return UQuestTextArgument::StaticClass(); }
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_QuestTextArg", "Quest Text Argument"); }
};

class FAssetTypeActions_QuestCustomData : public FAssetTypeActions_QuestObjectBase
{
public:

	FAssetTypeActions_QuestCustomData(EAssetTypeCategories::Type InAssetCategory) : FAssetTypeActions_QuestObjectBase(InAssetCategory) {}

	virtual UClass* GetSupportedClass() const override { return UQuestCustomData::StaticClass(); }
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_QuestCustomData", "Quest Custom Data"); }
};