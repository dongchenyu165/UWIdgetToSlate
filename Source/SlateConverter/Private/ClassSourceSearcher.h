#pragma once

struct FClassSourceFilesInfo
{
	UClass* ClassPtr;
	FString HeaderFilePath;
	FString SourceFilePath;

	// 将 FMyStruct 序列化到 Json 对象中
	void ToJson(TSharedPtr<FJsonObject>& JsonObject) const;

	// 从 Json 对象中反序列化 FMyStruct
	void FromJson(TSharedPtr<FJsonObject> JsonObject);
};

class FClassSourceSearcher
{
	using FFilesInfoCacheMapType = TMap<UClass*, FClassSourceFilesInfo>;
	static bool FindModuleBasePath(const FString& InModuleName, FString& OutModuleBasePath);
	static bool FindCPPSourceFile(const FString& InModuleName, const FString& InUClassModuleRelativePath);
	static bool LoadMapsFromJsonFile(const FString& FilePath);
	static bool SaveMapsToJsonFile(const FString& FilePath);

	static void BuildModulePathCacheMap();
public:
	static FString GetUClassModuleName(UClass* InClass);
	static bool FindUClassSourceFiles(UClass* InClass, FString& OutHeaderPath, FString& OutSourcePath);

private:
	inline static TMap<FString /* ModuleName */, FString /* ModuleBasePath */> ModulePathCacheMap;

	// Cache[MODULE_NAME][UCLASS_RELATIVE_PATH]
	inline static FFilesInfoCacheMapType ClassSourceFilesInfoCacheMap;
};
