#include "WidgetSourceScanner.h"

FString ExtractMemberName(FRegexMatcher& InMatcher)
{
	return InMatcher.GetCaptureGroup(3);
}

void FWidgetSourceScanner::LoadSourceFile(const FString& InSourceFilePath)
{
	FFileHelper::LoadFileToString(FileContent, *InSourceFilePath);
}

FString FWidgetSourceScanner::MatchingSlateMember(const FString& InClassName)
{
	if (!MatchedSlateMemberInfoList.IsEmpty())
	{
		return "";	
	}
	
	if (FileContent.IsEmpty())
	{
		FFileHelper::LoadFileToString(FileContent, *SourceFilePath);
	}
	const FString ClassHeaderPattern = FString::Printf(TEXT(R"(class\s+U%s\s*:\s*public)"), *InClassName);
	const FString ClassHeaderEndPattern = TEXT(R"(\};)");
	const FString FinalPtn = ClassHeaderPattern + TEXT(".*?") + ClassHeaderEndPattern;

	// Filter the class part
	FRegexPattern ClassCodePattern(ClassHeaderPattern);
	FRegexMatcher ClassCodeMatcher(ClassCodePattern, FileContent);
	ClassCodeMatcher.SetLimits(0, FileContent.Len());
	if (!ClassCodeMatcher.FindNext())
	{
		UE_LOG(LogTemp, Error, TEXT("No class found: %s"), *InClassName);
		return "";
	}

	FRegexMatcher ClassCodeEndMatcher(FRegexPattern(ClassHeaderEndPattern), FileContent);
	ClassCodeEndMatcher.SetLimits(ClassCodeMatcher.GetMatchBeginning(), FileContent.Len() + 10);
	if (!ClassCodeEndMatcher.FindNext())
	{
		UE_LOG(LogTemp, Error, TEXT("No class found: %s"), *InClassName);
		return "";
	}
	
	auto ClassCodeStart = ClassCodeMatcher.GetMatchBeginning();
	auto ClassCodeEnd = ClassCodeEndMatcher.GetMatchEnding();

	FRegexPattern SlateMemberPattern(SWidgetMemberMatchPattern);
	FRegexMatcher SlateMemberMatcher(SlateMemberPattern, FileContent);
	SlateMemberMatcher.SetLimits(ClassCodeStart, ClassCodeEnd);

	// Save all matches.
	MatchedSlateMemberInfoList.Empty();
	while (SlateMemberMatcher.FindNext())
	{
		MatchedSlateMemberInfoList.Add(FSlateMemberInfo{
			SlateMemberMatcher.GetCaptureGroup(1),
			SlateMemberMatcher.GetCaptureGroup(1).Contains("Ptr") ? "->" : ".",
			SlateMemberMatcher.GetCaptureGroup(2),
			SlateMemberMatcher.GetCaptureGroup(3)
		});
		// UE_LOG(LogTemp, Error, TEXT("\t Multiple Slate member found: [%s]"), *SlateMemberMatcher.GetCaptureGroup(3));
	}
	if (MatchedSlateMemberInfoList.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("No Slate member found in class: %s"), *InClassName);
		return "";
	}

	MatchedSlateMember = SlateMemberMatcher.GetCaptureGroup(3);
	MatchedSlateMemberAccessOperator = SlateMemberMatcher.GetCaptureGroup(1).Contains("Ptr") ? "->" : ".";
	return MatchedSlateMember;
}
