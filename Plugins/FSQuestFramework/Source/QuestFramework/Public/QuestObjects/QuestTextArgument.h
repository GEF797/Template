// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "QuestObjectBase.h"
#include "QuestTextArgument.generated.h"

class UQuestContext;

/**
 * Text argument to embed dynamic text into quest texts.
 * Derive a blueprint from this to implement custom text argument type.
 * This could be used to display item, character or place names etc.
 */
UCLASS(Blueprintable, EditInlineNew)
class QUESTFRAMEWORK_API UQuestTextArgument : public UQuestObjectBase
{
	GENERATED_BODY()

public:

	FText GetTextArgumentValue(const UQuestContext* Context) const;

protected:

	/**
	 * Override in blueprint or C++ to provide value for text argument
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Text Argument")
	FText GetText(UQuestContext* Context) const;
	virtual FText GetText_Implementation(UQuestContext* Context) const
	{
		return FText::FromString(FString(TEXT("NOT IMPLEMENTED!")));
	}
};
