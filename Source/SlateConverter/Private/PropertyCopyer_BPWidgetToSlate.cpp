#include "PropertyCopyer_BPWidgetToSlate.h"
#include "ClassSourceSearcher.h"
#include "Components/Widget.h"


/**
 * 将 InString 根据多个分隔符进行分割，结果存入 OutArray 中
 *
 * @param InString   待分割的字符串
 * @param Delimiters 分隔符集合，每个字符都作为一个分隔符
 * @param OutArray   分割后得到的字符串数组
 */
void SplitStringByMultipleDelimiters(const FString& InString, const FString& Delimiters, TArray<FString>& OutArray)
{
	OutArray.Empty();

	const int32 StringLength = InString.Len();
	int32 StartIndex = 0;

	for (int32 i = 0; i < StringLength; i++)
	{
		bool bIsDelimiter = false;
		// 检查当前字符是否为分隔符之一
		for (int32 j = 0; j < Delimiters.Len(); j++)
		{
			if (InString[i] == Delimiters[j])
			{
				bIsDelimiter = true;
				break;
			}
		}

		if (bIsDelimiter)
		{
			// 若存在非空子串则添加到数组中
			if (i > StartIndex)
			{
				OutArray.Add(InString.Mid(StartIndex, i - StartIndex));
			}
			// 更新起始索引至下一个字符
			StartIndex = i + 1;
		}
	}

	// 添加最后一段（如果不为空）
	if (StartIndex < StringLength)
	{
		OutArray.Add(InString.Mid(StartIndex));
	}
}

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
	// $2: Setter function name, $3: Setter function arguments
	FString Pattern_Full = TEXT(R"(()SafeWidget->Set(\w+)\(((?:[^()]+|\((?:[^()]+|\((?:[^()]+|\([^()]*\))*\))*\))*)\);)");

	const FRegexPattern RegexPattern(Pattern_Full);

	FRegexMatcher Matcher(RegexPattern, InFileContent);
	Matcher.SetLimits(0, InFileContent.Len());

	TMap<FString, TArray<FString>> PropertyNameToArgsStrList;

	while (Matcher.FindNext())
	{
		FString SlateAttrSetterFuncName = Matcher.GetCaptureGroup(2);
		TArray<FString> ArgsList;
		FString UWidgetParametersString = Matcher.GetCaptureGroup(3).Replace(TEXT(" "), TEXT(""));

		SplitStringByMultipleDelimiters(UWidgetParametersString, TEXT(".<>(), "), ArgsList);
		for (auto SymbolStr : ArgsList)
		{
			if (InClass->FindPropertyByName(FName(*SymbolStr)))
			{
				FPropertyMappingInfo& MappingInfo = Mapping[InClass->GetName()].Add(
					SymbolStr, FPropertyMappingInfo());
				MappingInfo.WidgetClass = InClass;
				// MappingInfo.WidgetPropertyStr = SymbolStr;
				// MappingInfo.SlateMemberIndex = -1;
				MappingInfo.SlatePropSetterStr = SlateAttrSetterFuncName;
				MappingInfo.SetterArgsStr = {SymbolStr};
			}
		}

		
		// UWidgetParametersString.ParseIntoArray(ArgsList, TEXT("<>(), "));
		// PropertyNameToArgsStrList.Add(SlateAttrSetterFuncName.Replace(TEXT("Set"), TEXT("")), ArgsList);

		
		// UWidgetParametersString.ParseIntoArray(ArgsList, TEXT(","));

		// for (auto& UWidgetPropertyVarName : ArgsList)
		// {
		// 	FPropertyMappingInfo& MappingInfo = Mapping[InClass->GetName()].Add(
		// 		UWidgetPropertyVarName, FPropertyMappingInfo());
		// 	MappingInfo.WidgetClass = InClass;
		// 	MappingInfo.WidgetPropertyStr = UWidgetPropertyVarName;
		// 	MappingInfo.SlateMemberIndex = -1;
		// 	MappingInfo.SlatePropSetterStr = Matcher.GetCaptureGroup(2);
		// 	MappingInfo.SetterArgsStr = ArgsList;
		// }
	}
	
	// FString Pattern = TEXT(R"(()SafeWidget->Set([\w,\d,_]*)\(([\w,\d,_]*)\))");
	// MatchSlateSetterByPattern(InClass, InFileContent, Pattern_Full);
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
