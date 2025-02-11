#pragma once

struct FClassSourceFilesInfo
{
	UClass* ClassPtr;
	FString HeaderFilePath;
	FString SourceFilePath;
};

class FClassSourceSearcher
{
	static bool FindModuleBasePath(const FString& InModuleName, FString& OutModuleBasePath);
	static bool FindCPPSourceFile(const FString& InModuleName, const FString& InUClassModuleRelativePath);

public:
	static FString GetUClassModuleName(UClass* InClass);
	static bool FindUClassSourceFiles(UClass* InClass, FString& OutHeaderPath, FString& OutSourcePath);

private:
inline 	static TMap<FString /* ModuleName */, FString /* ModuleBasePath */> ModulePathCacheMap;

	// Cache[MODULE_NAME][UCLASS_RELATIVE_PATH]
inline 	static TMap<UClass*, FClassSourceFilesInfo>	ClassSourceFilesInfoCacheMap;
};
