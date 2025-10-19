// Copyright Sami Kangasmaa, 2022


#include "AssetActions/AssetTypeActions_Quest.h"
#include "QuestFrameworkEditorModule.h"
#include "QuestEditor/QuestEditor.h"

void FAssetTypeActions_Quest::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (UObject* Obj : InObjects)
	{
		if (UQuest* Quest = Cast<UQuest>(Obj))
		{
			UE_LOG(LogQuestFrameworkEditor, Log, TEXT("Opening editor for quest asset in path %s"), *Quest->GetPathName());

			TSharedRef<FQuestEditor> QuestEditor(new FQuestEditor());
			QuestEditor->InitQuestEditor(Mode, EditWithinLevelEditor, Quest);
		}
	}
}
