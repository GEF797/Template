// Copyright, Sami Kangasmaa 2022

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "QuestData.h"
#include "QuestMemory.h"

#include "QuestTextTypes.generated.h"

class UQuestContext;
class UQuestTextArgument;

/**
 * Type of a text argument
 */
UENUM(BlueprintType)
enum class EQuestTextArgType : uint8
{
	// Replaces text with value of an variable value
	QuestVariable = 0,

	// Use QuestTextArgument object to implement custom logic
	Custom,
};

/**
 * Formatting options for quest text when formatting numeric values.
 * Converted into FNumberFormattingOptions (see Text.h)
 */
USTRUCT(BlueprintType)
struct QUESTFRAMEWORK_API FQuestNumberFormattingOptions
{
	GENERATED_BODY()

public:

	FQuestNumberFormattingOptions()
		: RoundingMode(ERoundingMode::HalfFromZero)
		, bUseGrouping(false)
		, MinimumIntegralDigits(FNumberFormattingOptions::DefaultNoGrouping().MinimumIntegralDigits)
		, MaximumIntegralDigits(FNumberFormattingOptions::DefaultNoGrouping().MaximumIntegralDigits)
		, MinimumFractionalDigits(0)
		, MaximumFractionalDigits(0)
	{}

	// The rounding mode to be used when the actual value can not be precisely represented due to restrictions on the number of integral or fractional digits. See values for details.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Numeral Formating")
	TEnumAsByte<ERoundingMode> RoundingMode;

	// Should the numerals use group separators. IE: "1,000,000"
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Numeral Formating")
	bool bUseGrouping;

	// How many integral digits should be shown at minimum? May cause digit "padding". IE: A minimum of 3 integral digits means 1.0 -> "001".
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Numeral Formating", meta = (ClampMin = "0"))
	int32 MinimumIntegralDigits;

	// How many integral digits should be shown at maximum? May cause rounding. IE: A maximum of 2 integral digits means 100.0 -> "99".
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Numeral Formating", meta = (ClampMin = "0"))
	int32 MaximumIntegralDigits;

	// How many fractional digits should be shown at minimum? May cause digit "padding". IE: A minimum of 2 fractional digits means 1.0 -> "1.00".
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Numeral Formating", meta = (ClampMin = "0"))
	int32 MinimumFractionalDigits;

	// How many fractional digits should be shown at maximum? May cause rounding. IE: HalfFromZero rounding and a maximum of 2 fractional digits means 0.009 -> "0.01".
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Numeral Formating", meta = (ClampMin = "0"))
	int32 MaximumFractionalDigits;

public:

	// Convers this into actual number formatting options
	FNumberFormattingOptions GetNumberFormattingOptions() const
	{
		FNumberFormattingOptions Options = FNumberFormattingOptions();

		Options.SetUseGrouping(bUseGrouping).
			SetRoundingMode(RoundingMode).
			SetMinimumIntegralDigits(MinimumIntegralDigits).
			SetMaximumIntegralDigits(MaximumIntegralDigits).
			SetMinimumFractionalDigits(MinimumFractionalDigits).
			SetMaximumFractionalDigits(MaximumFractionalDigits);

		return Options;
	}
};

/**
 * Text argument that can be injected into QuestTexts
 */
USTRUCT(BlueprintType)
struct QUESTFRAMEWORK_API FQuestTextArg
{
	GENERATED_BODY()

public:

	// Name of the argument to set in QuestText
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Text Argument")
	FString ArgName;

	// Type of this argument
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text Argument")
	EQuestTextArgType Type = EQuestTextArgType::QuestVariable;

	// Variable to read value from
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text Argument")
	FQuestVariable Variable;

	// Should the variable value conversed via number formatter?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text Argument")
	bool bUseNumberFormatter = false;

	// Options when formatting number variables to text
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text Argument")
	FQuestNumberFormattingOptions NumberFormatOptions;

	// Instanced object to run custom logic to get the argument value
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite, Category = "Text Argument")
	 TObjectPtr<UQuestTextArgument> CustomTextArgument = nullptr;

public:

	// Returns format argument that can be injected into format text
	FFormatArgumentValue GetArgumentValue(const UQuestContext* Context) const;
};

// Specific text field for quest texts to inject text arguments
USTRUCT(BlueprintType)
struct QUESTFRAMEWORK_API FQuestText
{
	GENERATED_BODY()

public:

	// Unformatted text
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Text", meta = (MultiLine = "true"))
	FText Text;

	// Arguments for text to format the text
	UPROPERTY(EditAnywhere, EditFixedSize, Category = "Quest Text")
	TArray<FQuestTextArg> TextArguments;

public:

	FQuestText()
	{
		Text = FText::GetEmpty();
	}

	// Returns unformatted text as it's written in the editor
	FText GetUnformattedText() const { return Text; }

	// Returns text that has been formatted using text arguments
	FText GetFormattedText(const UQuestContext* Context) const;

	/*
	* Updates TextArguments from Text property parsing all arguments
	*/
	void UpdateArgumentList();

#if WITH_EDITOR

	// Called when editing quest text properties to update the data
	void NotifyEditText();

	static FName GetTextPropertyName()
	{
		return GET_MEMBER_NAME_CHECKED(FQuestText, Text);
	}

#endif
};
