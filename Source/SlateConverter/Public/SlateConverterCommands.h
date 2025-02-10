// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"
#include "SlateConverterStyle.h"

class FSlateConverterCommands : public TCommands<FSlateConverterCommands>
{
public:

	FSlateConverterCommands()
		: TCommands<FSlateConverterCommands>(TEXT("SlateConverter"), NSLOCTEXT("Contexts", "SlateConverter", "SlateConverter Plugin"), NAME_None, FSlateConverterStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
