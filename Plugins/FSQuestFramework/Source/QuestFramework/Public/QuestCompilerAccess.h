// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"


/**
 * Interface to call quest compiler to compile quest via editor module from quest class
 */
class QUESTFRAMEWORK_API IQuestCompilerAccess
{
public:

	virtual void CompileQuestInEditorModule(class UQuest* Quest) = 0;
};
