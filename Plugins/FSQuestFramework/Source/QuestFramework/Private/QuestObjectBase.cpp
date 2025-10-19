// Copyright Sami Kangasmaa, 2022


#include "QuestObjectBase.h"
#include "UObject/Object.h"
#include "Engine/World.h"

UWorld* UQuestObjectBase::GetWorld() const
{
	if (HasAnyFlags(RF_ArchetypeObject | RF_ClassDefaultObject))
	{
		return nullptr;
	}

	// Get world that was set for function call scope. This is needed when non-instanced objects run events, so they have a correct world
	if (CallScopeWorldPtr)
	{
		return CallScopeWorldPtr;
	}

	// Get from outer
	if (UObject* Outer = GetOuter())
	{
		if (UWorld* World = Outer->GetWorld())
		{
			return World;
		}
	}

	return nullptr;
}

