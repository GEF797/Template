// Copyright Sami Kangasmaa, 2022

#include "QuestFrameworkExtensionModule.h"

#define LOCTEXT_NAMESPACE "QuestFrameworkExtension"

DEFINE_LOG_CATEGORY(LogQuestFrameworkExtension)

void FQuestFrameworkExtensionModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FQuestFrameworkExtensionModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FQuestFrameworkExtensionModule, QuestFrameworkExtension)