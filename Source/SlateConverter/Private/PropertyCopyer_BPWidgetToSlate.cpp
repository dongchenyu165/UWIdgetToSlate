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
