// Copyright Sami Kangasmaa, 2022

#include "QuestFrameworkModule.h"
#include "QuestFrameworkSettings.h"
#include "GameplayTagsManager.h"
#include "Misc/Paths.h"

#define LOCTEXT_NAMESPACE "QuestFramework"

DEFINE_LOG_CATEGORY(LogQuestFramework)

void FQuestFrameworkModule::StartupModule()
{
	// Load settings
	const UQuestFrameworkSettings* Settings = GetDefault<UQuestFrameworkSettings>();

	UGameplayTagsManager::Get().AddTagIniSearchPath(FPaths::ProjectPluginsDir() / TEXT("FSQuestFramework/Config/Tags"));
}

void FQuestFrameworkModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FQuestFrameworkModule, QuestFramework)