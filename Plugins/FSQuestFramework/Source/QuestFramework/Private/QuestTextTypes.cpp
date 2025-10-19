// Copyright, Sami Kangasmaa 2022


#include "QuestTextTypes.h"
#include "QuestContext.h"
#include "QuestMemory.h"
#include "QuestObjects/QuestTextArgument.h"

// -------------------------------------------------------------------------- //

FFormatArgumentValue FQuestTextArg::GetArgumentValue(const UQuestContext* Context) const
{
	if (!Context)
	{
		return FFormatArgumentValue(FText::FromString(TEXT("INVALID! MISSING QUEST CONTEXT!")));
	}

	if (Type == EQuestTextArgType::QuestVariable)
	{
		if (Variable.Type == EQuestVariableType::Integer)
		{
			const UQuestMemory* Memory = Context->GetQuestMemory();
			if (ensure(Memory))
			{
				const int32 Value = Memory->GetInteger(Variable.Variable);
				if (bUseNumberFormatter)
				{
					FNumberFormattingOptions Options = NumberFormatOptions.GetNumberFormattingOptions();
					return FFormatArgumentValue(FText::AsNumber(Value, &Options));
				}
				else
				{
					return FFormatArgumentValue(Value);
				}
			}
		}
		else if (Variable.Type == EQuestVariableType::Float)
		{
			const UQuestMemory* Memory = Context->GetQuestMemory();
			if (ensure(Memory))
			{
				const float Value = Memory->GetFloat(Variable.Variable);
				if (bUseNumberFormatter)
				{
					FNumberFormattingOptions Options = NumberFormatOptions.GetNumberFormattingOptions();
					return FFormatArgumentValue(FText::AsNumber(Value, &Options));
				}
				else
				{
					return FFormatArgumentValue(Value);
				}
			}
		}
		else if (Variable.Type == EQuestVariableType::Bool)
		{
			const UQuestMemory* Memory = Context->GetQuestMemory();
			if (ensure(Memory))
			{
				const int32 Value = (int32)Memory->GetBool(Variable.Variable);
				if (bUseNumberFormatter)
				{
					FNumberFormattingOptions Options = NumberFormatOptions.GetNumberFormattingOptions();
					return FFormatArgumentValue(FText::AsNumber(Value, &Options));
				}
				else
				{
					return FFormatArgumentValue(Value);
				}
			}
		}
	}
	else if (Type == EQuestTextArgType::Custom)
	{
		if (CustomTextArgument)
		{
			return FFormatArgumentValue(CustomTextArgument->GetTextArgumentValue(Context));
		}

		return FFormatArgumentValue(FText::FromString(TEXT("INVALID! CUSTOM ARGUMENT OBJECT MISSING!")));
	}

	ensure(false && "GetArgumentValue: Not all cases were handled correctly in FQuestTextArg::GetArgumentValue!");
	return FFormatArgumentValue(0);
}

// -------------------------------------------------------------------------- //

FText FQuestText::GetFormattedText(const UQuestContext* Context) const
{
	if (TextArguments.Num() <= 0)
	{
		return GetUnformattedText();
	}

	FFormatNamedArguments Arguments;
	for (const FQuestTextArg& Arg : TextArguments)
	{
		Arguments.Add(Arg.ArgName, Arg.GetArgumentValue(Context));
	}
	return FText::AsCultureInvariant(FText::Format(Text, Arguments));
}

void FQuestText::UpdateArgumentList()
{
	TArray<FQuestTextArg> TempArguments = TextArguments;
	TextArguments.Empty();

	TArray<FString> TextParams;
	FText::GetFormatPatternParameters(Text, TextParams);
	for (const FString& ParamName : TextParams)
	{
		FQuestTextArg Arg;
		Arg.ArgName = ParamName;

		for (const FQuestTextArg& ExistingArg : TempArguments)
		{
			if (ParamName == ExistingArg.ArgName)
			{
				Arg = ExistingArg;
				break;
			}
		}

		TextArguments.Add(Arg);
	}
}

#if WITH_EDITOR
void FQuestText::NotifyEditText()
{
	UpdateArgumentList();
}
#endif