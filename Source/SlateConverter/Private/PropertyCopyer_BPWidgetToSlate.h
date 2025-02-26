#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"

class FPropertyMappingInfo
{
	constexpr static int INVALID_INDEX = -1;

public:
	UClass* WidgetClass = nullptr;
	// The property name of the UWidget object
	FString WidgetPropertyStr;

	int SlateMemberIndex = INVALID_INDEX;

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
