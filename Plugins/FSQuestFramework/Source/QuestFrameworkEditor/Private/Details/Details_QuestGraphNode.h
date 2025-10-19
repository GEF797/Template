// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"


class FDetails_QuestGraphNode : public IDetailCustomization
{
	typedef FDetails_QuestGraphNode Self;

public:

	static TSharedRef<IDetailCustomization> MakeInstance() { return MakeShared<Self>(); }
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};

class FDetails_QuestGraphNode_Connection : public IDetailCustomization
{
	typedef FDetails_QuestGraphNode_Connection Self;

public:

	static TSharedRef<IDetailCustomization> MakeInstance() { return MakeShared<Self>(); }
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
