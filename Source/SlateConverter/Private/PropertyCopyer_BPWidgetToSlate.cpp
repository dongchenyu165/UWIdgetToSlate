#include "PropertyCopyer_BPWidgetToSlate.h"
#include "ClassSourceSearcher.h"
#include "Components/Widget.h"


FPropertyMappingInfo& FPropertyCopyer_BPWidgetToSlate::GetMappingInfo(UClass* InClass, const FString& InPropertyName)
{
	if (InClass == nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("Invalid UClass pointer."));
		return InvalidMappingInfo;
	}
	if (!InClass->IsChildOf(UWidget::StaticClass()))
	{
		UE_LOG(LogTemp, Log, TEXT("The class is not a child of UWidget."));
		return InvalidMappingInfo;
	}
	auto* p = &Mapping;
	FString ClassName = InClass->GetName();

	if (!Mapping.Contains(InClass->GetName()))
	{
		UClass* BuildingClassPtr = InClass;
		while (true)
		{
			if (!BuildingClassPtr)
			{
				break;
			}
			if (Mapping.Contains(BuildingClassPtr->GetName()))
			{
				BuildingClassPtr = BuildingClassPtr->GetSuperClass();
				continue;
			}

			BuildMapping(BuildingClassPtr);

			if (BuildingClassPtr == UWidget::StaticClass())
			{
				break;
			}

			BuildingClassPtr = BuildingClassPtr->GetSuperClass();
		}
	}
	if (!Mapping.Contains(InClass->GetName()))
	{
		UE_LOG(LogTemp, Log, TEXT("The class '%s' is not in the mapping."), *InClass->GetName());
		return InvalidMappingInfo;
	}

	auto FindPropertyInMapping = [&InPropertyName](UClass* ClassPtr) -> FPropertyMappingInfo*
	{
		while (ClassPtr)
		{
			if (Mapping[ClassPtr->GetName()].Contains(InPropertyName))
			{
				return &Mapping[ClassPtr->GetName()][InPropertyName];
			}

			if (ClassPtr == UWidget::StaticClass())
			{
				break;
			}
			ClassPtr = ClassPtr->GetSuperClass();
		}
		return nullptr;
	};

	FPropertyMappingInfo* FoundMappingInfo = FindPropertyInMapping(InClass);
	if (FoundMappingInfo)
	{
		return *FoundMappingInfo;
	}

	BuildMapping(InClass);

	FoundMappingInfo = FindPropertyInMapping(InClass);
	if (FoundMappingInfo)
	{
		return *FoundMappingInfo;
	}
	Mapping[InClass->GetName()].Add(InPropertyName, FPropertyMappingInfo());

	UE_LOG(LogTemp, Log, TEXT("The property '%s' is not in the mapping of class '%s'."), *InPropertyName, *InClass->GetName());
	return Mapping[InClass->GetName()][InPropertyName];
}

void FPropertyCopyer_BPWidgetToSlate::MatchingUWidgetSlateSetter(UClass* InClass, const FString& InFileContent)
{
	FString Pattern_Full = TEXT(R"(()SafeWidget->Set(\w+)\(((?:[^()]+|\((?:[^()]+|\((?:[^()]+|\([^()]*\))*\))*\))*)\);)");
	FString Pattern = TEXT(R"(()SafeWidget->Set([\w,\d,_]*)\(([\w,\d,_]*)\))");
	MatchSlateSetterByPattern(InClass, InFileContent, Pattern);
}

void FPropertyCopyer_BPWidgetToSlate::MatchingAllSlateSetter(UClass* InClass, const FString& InFileContent,
															 const FSlateMemberInfo& InSlateMemberInfo,
															 const int& InMemberInfoIndex)
{
	// FString PatternText = FString::Printf(
	// 	TEXT(R"((%s)(Set[\w,\d,_]*)\(([\w,\d,_]*)\))"), *(InSlateMemberInfo.MemberName + InSlateMemberInfo.AccessOp));

	// Remove [Set] prefix in the setter function name when matching group 2.
	// For example, [SetPadding] -> [Padding]
	// This is because the [SNew]'s chain initialization use the property name without the [Set] prefix.
	FString PatternText = FString::Printf(
		TEXT(R"((%s)Set([\w,\d,_]*)\(([\w,\d,_]*)\))"), *(InSlateMemberInfo.MemberName + InSlateMemberInfo.AccessOp));
	MatchSlateSetterByPattern(InClass, InFileContent, PatternText);
	for (auto MapInfo : Mapping[InClass->GetName()])
	{
		MapInfo.Value.SlateMemberIndex = InMemberInfoIndex;
	}
}

void FPropertyCopyer_BPWidgetToSlate::MatchSlateSetterByPattern(UClass* InClass, const FString& InFileContent,
	const FString& InPattern)
{
	const FRegexPattern RegexPattern(InPattern);

	FRegexMatcher Matcher(RegexPattern, InFileContent);
	Matcher.SetLimits(0, InFileContent.Len());

	while (Matcher.FindNext())
	{
		FString SlateAttrSetterFuncName = Matcher.GetCaptureGroup(2);
		TArray<FString> ArgsList;
		FString UWidgetParametersString = Matcher.GetCaptureGroup(3).Replace(TEXT(" "), TEXT(""));

		{
			
		}
		
		UWidgetParametersString.ParseIntoArray(ArgsList, TEXT(","));

		for (auto& UWidgetPropertyVarName : ArgsList)
		{
			FPropertyMappingInfo& MappingInfo = Mapping[InClass->GetName()].Add(
				UWidgetPropertyVarName, FPropertyMappingInfo());
			MappingInfo.WidgetClass = InClass;
			MappingInfo.WidgetPropertyStr = UWidgetPropertyVarName;
			MappingInfo.SlateMemberIndex = -1;
			MappingInfo.SlatePropSetterStr = Matcher.GetCaptureGroup(2);
			MappingInfo.SetterArgsStr = ArgsList;
		}
	}
}

void FPropertyCopyer_BPWidgetToSlate::BuildMapping(UClass* InClass)
{
	// InClass->GetSuperClass();
	// Find source code file path
	FString ClassHeaderPath, ClassSourcePath;
	FClassSourceSearcher::FindUClassSourceFiles(InClass, ClassHeaderPath, ClassSourcePath);

	// Load source code file content.
	FString FileContent;
	FFileHelper::LoadFileToString(FileContent, *ClassSourcePath);
	if (FileContent.IsEmpty())
	{
		UE_LOG(LogTemp, Display, TEXT("NO CPP FILE!! ;; Module: %s ;; Module path: %s ;; HEADER: %s"),
			   *InClass->GetName(), *ClassHeaderPath, *ClassHeaderPath);
		return;
	}

	// Create scanner for the UWidget class
	if (!ScannerMapping.Contains(InClass->GetName()))
	{
		ScannerMapping.Add(InClass->GetName(), FWidgetSourceScanner(ClassHeaderPath));
	}
	FWidgetSourceScanner& Scanner = ScannerMapping[InClass->GetName()];
	Scanner.MatchingSlateMember(InClass->GetName());

	if (!Mapping.Contains(InClass->GetName()))
	{
		Mapping.Add(InClass->GetName(), TMap<FString, FPropertyMappingInfo>());
	}
	if (InClass == UWidget::StaticClass())
	{
		MatchingUWidgetSlateSetter(InClass, FileContent);
	}
	else
	{
		for (int i = 0; i < Scanner.GetMatchedSlateMemberInfoList().Num(); ++i)
		{
			const FSlateMemberInfo& SlateMemberInfo = Scanner.GetMatchedSlateMemberInfoList()[i];

			MatchingAllSlateSetter(InClass, FileContent, SlateMemberInfo, i);
		}
	}
}
