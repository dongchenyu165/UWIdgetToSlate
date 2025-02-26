#include "WidgetToSlate.h"

#include "Components/Button.h"
#include "Components/Overlay.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"


FString WidgetToSlateStr(UWidget* InRootWidget)
{
	FString Code;

	if (!InRootWidget)
	{
		return "";
	}

	auto SlateWidgetRef = InRootWidget->TakeWidget();
	FString WidgetSlateName = SlateWidgetRef->GetTypeAsString();

	Code += FString::Printf(TEXT("SNew(%s)\n"), *WidgetSlateName);


	if (InRootWidget->IsA(UButton::StaticClass()))
	{
		UButton* Button = Cast<UButton>(InRootWidget);
		Button->GetContent();
		Code += TEXT("SNew(SButton)\n");
	}
	else if (InRootWidget->IsA(UTextBlock::StaticClass()))
	{
		UTextBlock* TextBlock = Cast<UTextBlock>(InRootWidget);

		FString TextValue = (TextBlock && TextBlock->GetText().IsEmpty() == false)
			                    ? TextBlock->GetText().ToString()
			                    : TEXT("Default Text");
		Code += FString::Printf(TEXT("SNew(STextBlock)\n	.Text(FText::FromString(TEXT(\"%s\")))\n"), *TextValue);
	}
	// 其它控件的转换逻辑...

	// 如果控件有子控件，需要递归处理
	if (UPanelWidget* Panel = Cast<UPanelWidget>(InRootWidget))
	{
		// Panel->slate
		// FString WidgetSlateName = Panel->TakeWidget()->GetTypeAsString();
		// Panel->TakeWidget()->GetChildren()->;
		Code += FString::Printf(TEXT("SNew(STextBlock)"));
		for (int32 i = 0; i < Panel->GetChildrenCount(); ++i)
		{
			UWidget* Child = Panel->GetChildAt(i);
			Code += WidgetToSlateStr(Child);
		}
	}

	return Code;
}

void AppendSlateProperty(FString& InOutCodeStr, UWidget* InWidget)
{
}

#include "Templates/Function.h"

// Updated CompareNormalProperty with an additional callback parameter.
bool CompareNormalProperty(FProperty* InProperty, void* ObjectA, void* ObjectB, int32 InDepth,
                           FUObjectCompareCallback OnDifferenceFound = [](FProperty*, void*, void*)
                           {
                           })
{
	FString Indent = FString::ChrN(InDepth * 2, TEXT(' '));
	// 布尔型
	if (FBoolProperty* BoolProperty = CastField<FBoolProperty>(InProperty))
	{
		bool ValueA = BoolProperty->GetPropertyValue_InContainer(ObjectA);
		bool ValueB = BoolProperty->GetPropertyValue_InContainer(ObjectB);

		if (ValueA != ValueB)
		{
			UE_LOG(LogTemp, Display, TEXT("NOEQUAL == Property '%s' differs: A = %s, B = %s"),
			       *InProperty->GetName(), ValueA ? TEXT("true") : TEXT("false"),
			       ValueB ? TEXT("true") : TEXT("false"));
			OnDifferenceFound(InProperty, ObjectA, ObjectB);
			return false;
		}
	}
	// 整数型
	else if (FNumericProperty* NumProperty = CastField<FNumericProperty>(InProperty))
	{
		FString ValueA = NumProperty->GetNumericPropertyValueToString(ObjectA);
		FString ValueB = NumProperty->GetNumericPropertyValueToString(ObjectB);
		// int32 ValueA = NumProperty->GetPropertyValue_InContainer(ObjectA);
		// int32 ValueB = NumProperty->GetPropertyValue_InContainer(ObjectB);

		if (ValueA != ValueB)
		{
			// UE_LOG(LogTemp, Display, TEXT("NOEQUAL == Property '%s' differs: A = %d, B = %d"),
			// *InProperty->GetName(), ValueA, ValueB);
			UE_LOG(LogTemp, Display, TEXT("NOEQUAL == Property '%s' differs: A = %s, B = %s"),
			       *InProperty->GetName(), *ValueA, *ValueB);
			OnDifferenceFound(InProperty, ObjectA, ObjectB);
			return false;
		}
	}
	// 字符串型
	else if (FStrProperty* StringProperty = CastField<FStrProperty>(InProperty))
	{
		FString ValueA = StringProperty->GetPropertyValue_InContainer(ObjectA);
		FString ValueB = StringProperty->GetPropertyValue_InContainer(ObjectB);

		if (ValueA != ValueB)
		{
			UE_LOG(LogTemp, Display, TEXT("NOEQUAL == Property '%s' differs: A = %s, B = %s"),
			       *InProperty->GetName(), *ValueA, *ValueB);
			OnDifferenceFound(InProperty, ObjectA, ObjectB);
			return false;
		}
	}
	// 名称型
	else if (FNameProperty* NameProperty = CastField<FNameProperty>(InProperty))
	{
		FName ValueA = NameProperty->GetPropertyValue_InContainer(ObjectA);
		FName ValueB = NameProperty->GetPropertyValue_InContainer(ObjectB);

		if (ValueA != ValueB)
		{
			UE_LOG(LogTemp, Display, TEXT("NOEQUAL == Property '%s' differs: A = %s, B = %s"),
			       *InProperty->GetName(), *ValueA.ToString(), *ValueB.ToString());
			OnDifferenceFound(InProperty, ObjectA, ObjectB);
			return false;
		}
	}
	// 文本型
	else if (FTextProperty* TextProperty = CastField<FTextProperty>(InProperty))
	{
		FText ValueA = TextProperty->GetPropertyValue_InContainer(ObjectA);
		FText ValueB = TextProperty->GetPropertyValue_InContainer(ObjectB);

		if (!ValueA.EqualTo(ValueB))
		{
			UE_LOG(LogTemp, Display, TEXT("NOEQUAL == Property '%s' differs: A = %s, B = %s"),
			       *InProperty->GetName(), *ValueA.ToString(), *ValueB.ToString());
			OnDifferenceFound(InProperty, ObjectA, ObjectB);
			return false;
		}
	}

	return true;
}

bool CompareProperty(FProperty* InProperty, void* ObjectA, void* ObjectB, int32 InDepth,
                     FUObjectCompareCallback OnDifferenceFound = [](FProperty*, void*, void*)
                     {
                     })
{
	FString Indent = FString::ChrN(InDepth * 2, TEXT(' '));
	// 对象引用
	if (FObjectProperty* ObjectProperty = CastField<FObjectProperty>(InProperty))
	{
		UObject* ValueA = ObjectProperty->GetPropertyValue_InContainer(ObjectA);
		UObject* ValueB = ObjectProperty->GetPropertyValue_InContainer(ObjectB);

		// Invalid object pointer
		if (!ValueA || !ValueB)
		{
			return ValueA == ValueB;  // Both are null, return true.
		}
		// Different object pointers
		if (ValueA != ValueB)
		{
			if (CompareUObjects(ValueA, ValueB, InDepth + 1, OnDifferenceFound) == false)
			{
				UE_LOG(LogTemp, Display, TEXT("NOEQUAL == Property '%s' differs: A = %s, B = %s"),
					  *InProperty->GetName(),
					  ValueA ? *ValueA->GetName() : TEXT("null"),
					  ValueB ? *ValueB->GetName() : TEXT("null"));
				OnDifferenceFound(InProperty, ObjectA, ObjectB);
				return false;
			}
		}
		return true;
	}
	// 类引用
	else if (FClassProperty* ClassProperty = CastField<FClassProperty>(InProperty))
	{
		UClass* ValueA = static_cast<UClass*>(ClassProperty->GetPropertyValue_InContainer(ObjectA));
		UClass* ValueB = static_cast<UClass*>(ClassProperty->GetPropertyValue_InContainer(ObjectB));

		if (!ValueA || !ValueB)
		{
			return ValueA == ValueB;  // Both are null, return true.
		}
		if (ValueA != ValueB)
		{
			UE_LOG(LogTemp, VeryVerbose, TEXT("%sProperty '%s' differs: A = %s, B = %s"),
			       *Indent, *InProperty->GetName(),
			       ValueA ? *ValueA->GetName() : TEXT("null"),
			       ValueB ? *ValueB->GetName() : TEXT("null"));
			OnDifferenceFound(InProperty, ObjectA, ObjectB);
			return false;
		}
	}
	// 数组
	else if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(InProperty))
	{
		FScriptArrayHelper ArrayHelperA(ArrayProperty, ArrayProperty->ContainerPtrToValuePtr<void>(ObjectA));
		FScriptArrayHelper ArrayHelperB(ArrayProperty, ArrayProperty->ContainerPtrToValuePtr<void>(ObjectB));

		int32 NumA = ArrayHelperA.Num();
		int32 NumB = ArrayHelperB.Num();

		if (NumA != NumB)
		{
			UE_LOG(LogTemp, Display, TEXT("NOEQUAL == Property '%s' differs in array size: A = %d, B = %d"),
			       *InProperty->GetName(), NumA, NumB);
			OnDifferenceFound(InProperty, ObjectA, ObjectB);
			return false;
		}

		FProperty* InnerProperty = ArrayProperty->Inner;
		for (int32 Index = 0; Index < NumA; ++Index)
		{
			void* ElementAPtr = ArrayHelperA.GetRawPtr(Index);
			void* ElementBPtr = ArrayHelperB.GetRawPtr(Index);
			CompareProperty(InnerProperty, ElementAPtr, ElementBPtr, InDepth + 1, OnDifferenceFound);
		}
	}
	// 结构体
	else if (FStructProperty* StructProperty = CastField<FStructProperty>(InProperty))
	{
		UStruct* StructA = StructProperty->ContainerPtrToValuePtr<UStruct>(ObjectA);
		UStruct* StructB = StructProperty->ContainerPtrToValuePtr<UStruct>(ObjectB);

		if (StructA && StructB)
		{
			for (TFieldIterator<FProperty> StructFieldIt(StructProperty->Struct); StructFieldIt; ++StructFieldIt)
			{
				if (!CompareProperty(*StructFieldIt, StructA, StructB, InDepth + 1, OnDifferenceFound))
				{
					return false;
				}
			}
		}
	}
	else
	{
		return CompareNormalProperty(InProperty, ObjectA, ObjectB, InDepth + 1, OnDifferenceFound);
	}
	return true;
}

bool CompareUObjects(UObject* ObjectA, UObject* ObjectB, int InDepth,
                     FUObjectCompareCallback OnDifferenceFound, bool bSubPropertyCallback)
{
	static FUObjectCompareCallback EmptyCallback = [](FProperty*, void*, void*) {};
	if (!ObjectA || !ObjectB || ObjectA->GetClass() != ObjectB->GetClass())
	{
		UE_LOG(LogTemp, Warning, TEXT("Objects are null or not of the same class."));
		return false;
	}
	if (!bSubPropertyCallback)
	{
		OnDifferenceFound = EmptyCallback;
	}

	UClass* ObjectClass = ObjectA->GetClass();
	FString Indent = FString::ChrN(InDepth * 2, TEXT(' '));
	
	UE_LOG(LogTemp, Display, TEXT(" ======= %s ======== >>>>>>>>>>>>>>>>"), *ObjectClass->GetName());

	bool bResult = true;
	for (TFieldIterator<FProperty> PropertyIt(ObjectClass); PropertyIt; ++PropertyIt)
	{
		FProperty* Property = *PropertyIt;
		bool bPropertyEqu = CompareProperty(Property, ObjectA, ObjectB, InDepth, EmptyCallback);
		bResult = bResult && bPropertyEqu;  // If any property is different, [bResult] set to false.
		UE_LOG(LogTemp, Display, TEXT("%s Iterate Property [%s]"), *Indent, *Property->GetName());
		
		if (!bPropertyEqu)
		{
			OnDifferenceFound(Property, ObjectA, ObjectB);
		}
	}
	UE_LOG(LogTemp, Display, TEXT(" -------- %s -------- <<<<<<<<<<<<<<<<<"), *ObjectClass->GetName());

	return bResult;
}


#if false

void DEP()
{
			// 对象引用
		if (FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
		{
			UObject* ValueA = ObjectProperty->GetPropertyValue_InContainer(ObjectA);
			UObject* ValueB = ObjectProperty->GetPropertyValue_InContainer(ObjectB);

			if (ValueA != ValueB)
			{
				UE_LOG(LogTemp, Display, TEXT("NOEQUAL == Property '%s' differs: A = %s, B = %s"),
					   *Property->GetName(),
					   ValueA ? *ValueA->GetName() : TEXT("null"),
					   ValueB ? *ValueB->GetName() : TEXT("null"));
			}
		}
		// 类引用
		else if (FClassProperty* ClassProperty = CastField<FClassProperty>(Property))
		{
			UClass* ValueA = static_cast<UClass*>(ClassProperty->GetPropertyValue_InContainer(ObjectA));
			UClass* ValueB = static_cast<UClass*>(ClassProperty->GetPropertyValue_InContainer(ObjectB));

			if (ValueA != ValueB)
			{
				UE_LOG(LogTemp, Display, TEXT("NOEQUAL == Property '%s' differs: A = %s, B = %s"),
					   *Property->GetName(),
					   ValueA ? *ValueA->GetName() : TEXT("null"),
					   ValueB ? *ValueB->GetName() : TEXT("null"));
			}
		}
		// 数组
		else if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
		{
			FScriptArrayHelper ArrayHelperA(ArrayProperty, ArrayProperty->ContainerPtrToValuePtr<void>(ObjectA));
			FScriptArrayHelper ArrayHelperB(ArrayProperty, ArrayProperty->ContainerPtrToValuePtr<void>(ObjectB));

			int32 NumA = ArrayHelperA.Num();
			int32 NumB = ArrayHelperB.Num();

			if (NumA != NumB)
			{
				UE_LOG(LogTemp, Display, TEXT("NOEQUAL == Property '%s' differs in array size: A = %d, B = %d"),
					   *Property->GetName(), NumA, NumB);
				continue;
			}

			for (int32 Index = 0; Index < NumA; ++Index)
			{
				FString ElementA, ElementB;
				ArrayProperty->Inner->ExportTextItem(ElementA, ArrayHelperA.GetRawPtr(Index), nullptr, nullptr,
													 PPF_None);
				ArrayProperty->Inner->ExportTextItem(ElementB, ArrayHelperB.GetRawPtr(Index), nullptr, nullptr,
													 PPF_None);

				if (ElementA != ElementB)
				{
					UE_LOG(LogTemp, Display, TEXT("NOEQUAL == Property '%s' differs at array index %d: A = %s, B = %s"),
						   *Property->GetName(), Index, *ElementA, *ElementB);
				}
			}
		}
		// 结构体
		else if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			void* StructA = StructProperty->ContainerPtrToValuePtr<void>(ObjectA);
			void* StructB = StructProperty->ContainerPtrToValuePtr<void>(ObjectB);

			if (StructA && StructB)
			{
				// 递归比较结构体字段
				for (TFieldIterator<FProperty> StructFieldIt(StructProperty->Struct); StructFieldIt; ++StructFieldIt)
				{
					FProperty* StructField = *StructFieldIt;
					// FString ValueA, ValueB;
					// StructField->ExportTextItem(ValueA, StructField->ContainerPtrToValuePtr<void>(StructA), nullptr,
					//                             nullptr, PPF_None);
					// StructField->ExportTextItem(ValueB, StructField->ContainerPtrToValuePtr<void>(StructB), nullptr,
					//                             nullptr, PPF_None);
					//
					// if (ValueA != ValueB)
					// {
					// 	UE_LOG(LogTemp, Display, TEXT("NOEQUAL == Property '%s.%s' differs: A = %s, B = %s"),
					// 	       *Property->GetName(), *StructField->GetName(), *ValueA, *ValueB);
					// }
					CompareUObjects(UObject* ObjectA, UObject* ObjectB, int InDepth);
				}
			}
		}
		else
		{
			CompareNormalProperty(Property, ObjectA, ObjectB, InDepth);
		}
}
#endif
