#pragma once

#include "CoreMinimal.h"

class FPropertyMappingInfo
{
	// The property name of the UWidget object
	FString WidgetPropertyStr;
	UClass* WidgetClass = nullptr;

	// The setter function name to set the property for Slate object
	FString SlatePropSetterStr;

	// The arguments of the setter function.
	// Each element is a string of the UPROPERTY in the [WidgetClass], which input into the setter function.
	TArray<FString> SetterArgsStr;
};

class FPropertyCopyer_BPWidgetToSlate
{
public:
	static void LoadWidgetToSlateMapping();
	static void SaveWidgetToSlateMapping();

	static void MakeMappingByScanSourceCode();

	static TMap<FString /* Widget Class Name */, TMap<FString /* Widget prop */, FPropertyMappingInfo>> Mapping;
};
