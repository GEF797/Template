// Copyright G.E.FC 2023

#include "UIForGAS.h"

#define LOCTEXT_NAMESPACE "FUIForGASModule"

void FUIForGASModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FUIForGASModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUIForGASModule, UIForGAS)