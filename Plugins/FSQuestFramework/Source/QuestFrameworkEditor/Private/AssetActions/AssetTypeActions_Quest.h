// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"
#include "Quest.h"

class IToolkitHost;

/**
 * Definition for asset actions for quest assets
 * (Create via menu etc.)
 */
class FAssetTypeActions_Quest : public FAssetTypeActions_Base
{
public:

	FAssetTypeActions_Quest(EAssetTypeCategories::Type InAssetCategory) : AssetCategory(InAssetCategory) {}

	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_Quest", "Quest"); }

	virtual FColor GetTypeColor() const override { return FColor::Blue; }

	virtual UClass* GetSupportedClass() const override { return UQuest::StaticClass(); }

	virtual bool HasActions(const TArray<UObject*>& InObjects) const override { return false; }

	virtual uint32 GetCategories() override { return AssetCategory; }

	bool CanFilter() override { return true; }

	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;

private:

	EAssetTypeCategories::Type AssetCategory;
};