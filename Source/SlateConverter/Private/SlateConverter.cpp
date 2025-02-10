// Copyright Epic Games, Inc. All Rights Reserved.

#include "SlateConverter.h"
#include "SlateConverterStyle.h"
#include "SlateConverterCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"
#include "UMGEditorModule.h"
#include "WidgetBlueprint.h"
#include "Widgets/SWidget.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "WidgetBlueprintEditor.h"

#include "Blueprint/WidgetTree.h"

static const FName SlateConverterTabName("SlateConverter");

#define LOCTEXT_NAMESPACE "FSlateConverterModule"

void FSlateConverterModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FSlateConverterStyle::Initialize();
	FSlateConverterStyle::ReloadTextures();

	FSlateConverterCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FSlateConverterCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FSlateConverterModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FSlateConverterModule::RegisterMenus));
}

void FSlateConverterModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FSlateConverterStyle::Shutdown();

	FSlateConverterCommands::Unregister();
}

void FSlateConverterModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FSlateConverterCommands::Get().PluginAction, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(
					FToolMenuEntry::InitToolBarButton(FSlateConverterCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}

		// 创建工具栏扩展器
		ToolbarExtender = MakeShareable(new FExtender());
		ToolbarExtender->AddToolBarExtension(
			"WidgetTools", // 扩展点名称
			EExtensionHook::After,
			nullptr,
			FToolBarExtensionDelegate::CreateRaw(this, &FSlateConverterModule::AddToolbarExtension)
		);
		IUMGEditorModule& UMGEditorModule = FModuleManager::LoadModuleChecked<IUMGEditorModule>("UMGEditor");
		UMGEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
}


void FSlateConverterModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	// 在工具栏中添加一个按钮，设置标签、提示和图标
	Builder.AddToolBarButton(
		FUIAction(FExecuteAction::CreateRaw(this, &FSlateConverterModule::PluginButtonClicked)),
		NAME_None,
		LOCTEXT("MyButton_Label", "My Button"),
		LOCTEXT("MyButton_Tooltip", "点击执行自定义操作"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.GameSettings") // 此处可换成你需要的图标
	);
}

void FSlateConverterModule::PluginButtonClicked()
{
	// Put your "OnButtonClicked" stuff here
	UWidgetBlueprint* EditingUMG_Ptr = GetCurrentlyEditedAssets();
	if (!EditingUMG_Ptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoEditingUMG", "No UMG is currently being edited."));
		return;
	}

	
	
}

UUserWidget* FSlateConverterModule::GetCurrentlyEditedAssets()
{
	if (!GEditor)
	{
		return nullptr;
	}

	// 获取 AssetEditorSubsystem
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (!AssetEditorSubsystem)
	{
		return nullptr;
	}

	// 获取当前正在编辑的所有资产
	TArray<UObject*> EditedAssets = AssetEditorSubsystem->GetAllEditedAssets();

	for (UObject* Asset : EditedAssets)
	{
		if (!Asset || Asset->GetClass() != UUserWidget::StaticClass())
		{
			continue;
		}

		{
			// 打印正在编辑的资产名称
			UE_LOG(LogTemp, Log, TEXT("Currently Editing Asset: %s"), *Asset->GetName());

			return Cast<UUserWidget>(Asset);
		}
	}

	return nullptr;
}
FString FSlateConverterModule::ConvertWidgetTreeToSlate(UWidgetTree* InWidgetTree)
{
	FString SlateCode;
	if (!InWidgetTree)
	{
		return SlateCode;
	}

	InWidgetTree->RootWidget;

	// 遍历 WidgetTree，生成 Slate 代码
	InWidgetTree->ForEachWidget(
		[InWidgetTree](UWidget* InWidgetInTree)
		{
			if (!InWidgetInTree)
			{
				return;
			}

			// 生成 Slate 代码
			UE_LOG(LogTemp, Display, TEXT("Function:[%hs] Widget name: [%s] Slate name: [%s]"), __FUNCTION__, *InWidgetInTree->GetName(), *InWidgetInTree->TakeWidget()->GetTypeAsString());
			int Depth = 0;
			TObjectPtr<UWidget> WidgetParent = InWidgetInTree->GetParent();
			if (!WidgetParent)
			{
				return;
			}
			while (WidgetParent != InWidgetTree->RootWidget)
			{
				WidgetParent = WidgetParent->GetParent();
				Depth++;
			}
		}
	);

	return SlateCode;
}


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSlateConverterModule, SlateConverter)
