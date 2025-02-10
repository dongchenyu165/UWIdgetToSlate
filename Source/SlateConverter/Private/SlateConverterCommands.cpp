// Copyright Epic Games, Inc. All Rights Reserved.

#include "SlateConverterCommands.h"

#define LOCTEXT_NAMESPACE "FSlateConverterModule"

void FSlateConverterCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "SlateConverter", "Execute SlateConverter action", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
