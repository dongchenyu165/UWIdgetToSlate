#pragma once
#include "CoreMinimal.h"

struct FSlateMemberInfo
{
	FString SharedType;
	FString AccessOp;
	FString SlateTypeStr;
	FString MemberName;
};

class FWidgetSourceScanner
{
	constexpr static const TCHAR* DefaultSWidgetMemberMatchPattern = TEXT(
		// $1: Shared type, $2: Widget type, $3: Widget member name
		R"(TShared((?:Ptr|Ref))<\s*(?:class\s+)?(S\w+(?:\s*<.*?>)?)\s*>\s+(\w+)\s*;)");

public:
	FWidgetSourceScanner(const FString& InSourceFilePath,
	                     const FString& InSWidgetMemberMatchPattern = DefaultSWidgetMemberMatchPattern)
		: SourceFilePath(InSourceFilePath), SWidgetMemberMatchPattern(InSWidgetMemberMatchPattern)
	{
		LoadSourceFile(InSourceFilePath);
	}

	void LoadSourceFile(const FString& InSourceFilePath);
	/**
	 * 
	 * @param InClassName Scan class name, ONLY scan the class-part with this name
	 * @return Matched Slate member name
	 */
	FString MatchingSlateMember(const FString& InClassName);

	FString GetMatchPattern_SlateMemberAccessOp() const
	{
		return MatchedSlateMember + MatchedSlateMemberAccessOperator;
	}

	const TArray<FSlateMemberInfo>& GetMatchedSlateMemberInfoList()
	{
		return MatchedSlateMemberInfoList;
	}

protected:
	FString FileContent;
	FString SourceFilePath;

	FString SWidgetMemberMatchPattern = DefaultSWidgetMemberMatchPattern;

	TArray<FSlateMemberInfo> MatchedSlateMemberInfoList;
	FString MatchedSlateMember;
	FString MatchedSlateMemberAccessOperator = "->";
};
