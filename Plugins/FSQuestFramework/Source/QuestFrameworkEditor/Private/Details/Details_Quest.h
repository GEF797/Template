// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class FDetails_Quest : public IDetailCustomization
{
	typedef FDetails_Quest Self;

public:

	static TSharedRef<IDetailCustomization> MakeInstance() { return MakeShared<Self>(); }
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
