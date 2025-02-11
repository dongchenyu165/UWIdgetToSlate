#include "F:\EpicGames\SelfCompiled_UE5\Engine\Intermediate\Build\Win64\x64\UnrealEditorGPF\Development\UnrealEd\SharedPCH.UnrealEd.Project.ValApi.Cpp20.h"
#include "PropertyCopyer_BPWidgetToSlate.h"

#include "Components/Widget.h"




void FPropertyCopyer_BPWidgetToSlate::MakeMappingByScanSourceCode()
{
	// TODO: Make BasePath configurable
	FString BasePath = "F:/EpicGames/SelfCompiled_UE5/Engine/Source/Runtime/UMG";

		TArray<UClass*> DerivedClasses;
	GetDerivedClasses(UWidget::StaticClass(), DerivedClasses, true);
	int i = 0;
	for (auto It = DerivedClasses.CreateIterator(); It; ++It, ++i)
	{
		auto& ClassVar = *It;

		FString ClassHeaderPath;
		FString ClassSourcePath;
		bool bHasSourceFile = GetUClassSourceFiles(ClassVar, ClassHeaderPath, ClassSourcePath);

		FString FileContent;
		FFileHelper::LoadFileToString(FileContent, *ClassSourcePath);
		if (FileContent.IsEmpty())
		{
			UE_LOG(LogTemp, Display, TEXT("NO CPP FILE!! ;; Module: %s ;; Module path: %s ;; HEADER: %s"),
			       *ClassVar->GetName(), *ClassHeaderPath, *ClassHeaderPath);
			continue;
		}

		// FRegexPattern Pattern(TEXT(R"((My\w*)->(Set[\w,\d,_]*)\()"));
		const FRegexPattern Pattern(TEXT(R"((My\w*)->(Set[\w,\d,_]*)\(([\w,\d,_]*)\))"));
		FRegexMatcher Matcher(Pattern, FileContent);
		Matcher.SetLimits(0, FileContent.Len());

		while (Matcher.FindNext())
		{
			FString SlateWidgetVarName = Matcher.GetCaptureGroup(1);
			FString SlateAttrSetterFuncName = Matcher.GetCaptureGroup(2);
			TArray<FString> UWidgetParameterStringList;
			FString UWidgetParametersString = Matcher.GetCaptureGroup(3).Replace(TEXT(" "), TEXT(""));
			UWidgetParametersString.ParseIntoArray(UWidgetParameterStringList, TEXT(","));

			if (UWidgetParameterStringList.Num() > 1)
			{
				UE_LOG(LogTemp, Display, TEXT(" -=-=-=- Module: %s ;; SlateVar: %s ;; SlateSetter: %s ;; Params: %s"),
				       *ClassVar->GetName(), *SlateWidgetVarName, *SlateAttrSetterFuncName, *UWidgetParametersString);
			}
			else if (UWidgetParameterStringList.Num() == 1)
			{
				UE_LOG(LogTemp, Display, TEXT(" +++1+++ Module: %s ;; SlateVar: %s ;; SlateSetter: %s ;; Params: %s"),
				       *ClassVar->GetName(), *SlateWidgetVarName, *SlateAttrSetterFuncName, *UWidgetParametersString);
			}
			else
			{
				UE_LOG(LogTemp, Display,
				       TEXT(" +++ELSE+++ Module: %s ;; SlateVar: %s ;; SlateSetter: %s ;; Params: %s"),
				       *ClassVar->GetName(), *SlateWidgetVarName, *SlateAttrSetterFuncName, *UWidgetParametersString);
			}
		}

		if (i > 5)
		{
			break;
		}
	}
}
