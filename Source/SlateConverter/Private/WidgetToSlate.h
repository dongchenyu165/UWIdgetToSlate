#pragma once
#include "CoreMinimal.h"

using FUObjectCompareCallback = TFunction<void(FProperty*, void*, void*)>;

class UWidget;
inline FString WidgetToSlateStr(UWidget* InRootWidget);
// void CompareFPropertyValue(FProperty* InnerProp, void* ValueA, void* ValueB, int32 Depth);
// void CompareStructProperties(UScriptStruct* Struct, void* StructA, void* StructB, int32 Depth);
// void CompareUObjectProperties(UObject* ObjA, UObject* ObjB, int32 Depth = 0);
bool CompareUObjects(UObject* ObjectA, UObject* ObjectB, int InDepth = 0,
                     FUObjectCompareCallback Callback = [](FProperty*, void*, void*)
                     {
                     });
// void CompareUObjects(UStruct* ObjectA, UStruct* ObjectB, int InDepth = 0);
