// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "QuestObjectBase.generated.h"

struct FQuestObjectWorldScope;

/**
 * Base object for all quest framework objects that require a valid world.
 */
UCLASS(ClassGroup = "Quest", Abstract, HideCategories = ("DoNotShow"), AutoExpandCategories = ("Default"))
class QUESTFRAMEWORK_API UQuestObjectBase : public UObject
{
	GENERATED_BODY()

public:

	UWorld* GetWorld() const override;

private:

	friend struct FQuestObjectWorldScope;
	void SetScopeWorld(UWorld* World) const
	{
		CallScopeWorldPtr = World;
	}
	mutable UWorld* CallScopeWorldPtr = nullptr;
};

struct FQuestObjectWorldScope
{
private:

	const UQuestObjectBase* ObjectPtr = nullptr;
	FQuestObjectWorldScope() {}
	FQuestObjectWorldScope(const FQuestObjectWorldScope& Other) {}

public:

	FQuestObjectWorldScope(const UQuestObjectBase* Object, UWorld* World)
	{
		check(Object);
		check(World);
		ObjectPtr = Object;
		Object->SetScopeWorld(World);
	}

	~FQuestObjectWorldScope()
	{
		if (ObjectPtr)
		{
			ObjectPtr->SetScopeWorld(nullptr);
		}
	}
};