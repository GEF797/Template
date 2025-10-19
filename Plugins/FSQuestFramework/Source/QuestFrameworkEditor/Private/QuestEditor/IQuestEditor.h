// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "QuestCompilerAccess.h"
#include "Toolkits/AssetEditorToolkit.h"


class IQuestEditor : public FAssetEditorToolkit,
	public IQuestCompilerAccess
{
public:

	virtual void CompileQuestInEditorModule(class UQuest* Quest) { checkNoEntry(); }

};
