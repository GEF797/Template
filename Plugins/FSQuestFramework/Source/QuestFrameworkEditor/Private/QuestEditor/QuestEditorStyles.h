// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "Styling/StyleColors.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

/**
* Contains style related things for quest editor widgets 
*/
class FQuestEditorStyles
{
public:

	static FSlateColor InputPinColor;
	static FSlateColor InputPinColor_Hover;

	static FSlateColor OutputPinColor;
	static FSlateColor OutputPinColor_Hover;
	static FSlateColor OutputPinColor_Completed;
	static FSlateColor OutputPinColor_Failed;

	static FSlateColor ConnectionColor;
	static FSlateColor ConnectionColor_Conditions;
	static FSlateColor ConnectionColor_Hover;

	static FSlateColor CommonNodeBorderColor;

	static FLinearColor NodeBodyColor_Root;
	static FLinearColor NodeBodyColor_Stage;
	static FLinearColor NodeBodyColor_Selector;
	static FLinearColor NodeBodyColor_End_Complete;
	static FLinearColor NodeBodyColor_End_Failed;

	static FLinearColor NodeBodyColor_Objective;
	static FLinearColor NodeBodyColor_Objective_Optional;
	static FLinearColor NodeBodyColor_Objective_RequireComplete;
	static FLinearColor NodeBodyColor_Objective_ForceComplete;
	static FLinearColor NodeBodyColor_Objective_ForceFail;
	static FLinearColor NodeBodyColor_Objective_ForceCompleteFail;

	static FLinearColor NodeBodyColor_Service;

	static FLinearColor VariableLabelColor_Int;
	static FLinearColor VariableLabelColor_Float;
	static FLinearColor VariableLabelColor_Bool;

public:

	static void InitializeSlateStyles();
	static void UninitializeSlateStyles();

private:

	static TSharedPtr<FSlateStyleSet> SlateStyleSet;
};
