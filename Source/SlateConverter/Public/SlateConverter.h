// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class UWidgetBlueprint;
class FToolBarBuilder;
class FMenuBuilder;

class FSlateConverterModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command. */
	void PluginButtonClicked();
	
private:

	void RegisterMenus();
	void AddToolbarExtension(FToolBarBuilder& Builder);
	static UWidgetBlueprint* GetCurrentlyEditedAssets();

private:
	TSharedPtr<class FUICommandList> PluginCommands;
	TSharedPtr<class FExtender> ToolbarExtender;
};
