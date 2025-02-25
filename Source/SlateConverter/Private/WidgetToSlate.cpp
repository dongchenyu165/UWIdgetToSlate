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

void CompareNormalProperty(FProperty* InProperty, void* ObjectA, void* ObjectB, int32 InDepth)
                           FUObjectCompareCallback OnDifferenceFound = [](FProperty*, void*, void*)
                           {
                           })
{
	FString Indent = FString::ChrN(InDepth * 2, TEXT(' '));
	// UE_LOG(LogTemp, Display, TEXT("%sProperty '%s'"), *Indent, *InProperty->GetName());
	
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
			OnDifferenceFound(InProperty, ObjectA, ObjectB);
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
		}
	}
}

                     FUObjectCompareCallback OnDifferenceFound = [](FProperty*, void*, void*)
                     {
                     })
{
	FString Indent = FString::ChrN(InDepth * 2, TEXT(' '));
	UE_LOG(LogTemp, Display, TEXT("%sProperty '%s'"), *Indent, *InProperty->GetName());
	
	// 对象引用
	if (FObjectProperty* ObjectProperty = CastField<FObjectProperty>(InProperty))
	{
		UObject* ValueA = ObjectProperty->GetPropertyValue_InContainer(ObjectA);
		UObject* ValueB = ObjectProperty->GetPropertyValue_InContainer(ObjectB);

		if (!ValueA || !ValueB)
		{
			return;
		}
		if (ValueA != ValueB)
		{
			UE_LOG(LogTemp, Display, TEXT("NOEQUAL == Property '%s' differs: A = %s, B = %s"),
			       *InProperty->GetName(),
			       ValueA ? *ValueA->GetName() : TEXT("null"),
			       ValueB ? *ValueB->GetName() : TEXT("null"));
			OnDifferenceFound(InProperty, ObjectA, ObjectB);
		}
		
		CompareUObjects(ValueA, ValueB, InDepth + 1);
	}
	// 类引用
	else if (FClassProperty* ClassProperty = CastField<FClassProperty>(InProperty))
	{
		UClass* ValueA = static_cast<UClass*>(ClassProperty->GetPropertyValue_InContainer(ObjectA));
		UClass* ValueB = static_cast<UClass*>(ClassProperty->GetPropertyValue_InContainer(ObjectB));

		if (!ValueA || !ValueB)
		{
			return;
		}
		if (ValueA != ValueB)
		{
			UE_LOG(LogTemp, VeryVerbose, TEXT("%sProperty '%s' differs: A = %s, B = %s"),
			       *Indent, *InProperty->GetName(),
			       ValueA ? *ValueA->GetName() : TEXT("null"),
			       ValueB ? *ValueB->GetName() : TEXT("null"));
			OnDifferenceFound(InProperty, ObjectA, ObjectB);
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
			return;
		}

		FProperty* p = ArrayProperty->Inner;
		for (int32 Index = 0; Index < NumA; ++Index)
		{
			// 获取当前元素的原始数据指针
			void* ElementAPtr = ArrayHelperA.GetRawPtr(Index);
			void* ElementBPtr = ArrayHelperB.GetRawPtr(Index);
			// CompareUObjects(p, ObjectA, ObjectB, InDepth + 1);
			CompareProperty(p, ElementAPtr, ElementBPtr, InDepth + 1);
		}
	}
	// 结构体
	else if (FStructProperty* StructProperty = CastField<FStructProperty>(InProperty))
	{
		UStruct* StructA = StructProperty->ContainerPtrToValuePtr<UStruct>(ObjectA);
		UStruct* StructB = StructProperty->ContainerPtrToValuePtr<UStruct>(ObjectB);

		if (StructA && StructB)
		{
			// 递归比较结构体字段
			for (TFieldIterator<FProperty> StructFieldIt(StructProperty->Struct); StructFieldIt; ++StructFieldIt)
			{
				CompareProperty(*StructFieldIt, StructA, StructB, InDepth + 1);
			}
		}
	}
	else
	{
		CompareNormalProperty(InProperty, ObjectA, ObjectB, InDepth);
	}
}

void CompareUObjects(UObject* ObjectA, UObject* ObjectB, int InDepth,
                     FUObjectCompareCallback OnDifferenceFound)
{
	if (!ObjectA || !ObjectB || ObjectA->GetClass() != ObjectB->GetClass())
	{
		UE_LOG(LogTemp, Warning, TEXT("Objects are null or not of the same class."));
		return;
	}

	UClass* ObjectClass = ObjectA->GetClass();
	FString Indent = FString::ChrN(InDepth * 2, TEXT(' '));
	// FString::

	for (TFieldIterator<FProperty> PropertyIt(ObjectClass); PropertyIt; ++PropertyIt)
	{
		FProperty* Property = *PropertyIt;
		CompareProperty(Property, ObjectA, ObjectB, InDepth, OnDifferenceFound);
	}
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
