#include "ClassSourceSearcher.h"


TArray<FString> FindFilesInDirectory(const FString& Directory, const FString& FileExtension)
{
	TArray<FString> FoundFiles;

	if (FPaths::DirectoryExists(Directory))
	{
		// 获取文件管理器实例
		IFileManager& FileManager = IFileManager::Get();

		// 在目录中查找匹配扩展名的文件
		FileManager.FindFiles(FoundFiles, *FPaths::Combine(Directory, TEXT("*") + FileExtension), true, false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Directory does not exist: %s"), *Directory);
	}

	return FoundFiles;
}

bool FindFirstDirRecursively(const FString& Directory, const FString& InFolderName, FString& OutFirstDirPath)
{
	static TMap<FString /* InFolderName */, FString /* FirstDirPath */> DirCacheMap;
	if (!FPaths::DirectoryExists(Directory))
	{
		UE_LOG(LogTemp, Warning, TEXT("The input directory does not exist: %s"), *Directory);
		OutFirstDirPath = "";
		return false;
	}

	if (DirCacheMap.Contains(InFolderName))
	{
		OutFirstDirPath = DirCacheMap[InFolderName];
		return true;
	}

	// 遍历目录及其子目录
	IFileManager::Get().IterateDirectoryRecursively(
		*Directory, [&](const TCHAR* FilenameOrDirectory, bool bIsDirectory) -> bool
		{
			if (!bIsDirectory)
			{
				return true; // continue iterating
			}

			const FString LeafFolderName = FPaths::GetPathLeaf(FilenameOrDirectory);
			ensure(!DirCacheMap.Contains(LeafFolderName));
			DirCacheMap.Add(LeafFolderName, FilenameOrDirectory);

			if (LeafFolderName == InFolderName)
			{
				OutFirstDirPath = FilenameOrDirectory;
				return false;
			}
			return true; // continue iterating
		});

	return !OutFirstDirPath.IsEmpty();
}

bool FindFilesRecursively(const FString& Directory, const FString& InFileFullName, TArray<FString>& OutFiles)
{
	static TMap<FString /* InFileFullName */, TArray<FString> /* FullDirectory */> FileCacheMap;
	if (FPaths::DirectoryExists(Directory))
	{
		if (FileCacheMap.Find(InFileFullName))
		{
			OutFiles = FileCacheMap[InFileFullName];
			return true;
		}
		// 遍历目录及其子目录
		IFileManager::Get().IterateDirectoryRecursively(
			*Directory, [&](const TCHAR* FilenameOrDirectory, bool bIsDirectory) -> bool
			{
				// 如果是文件并且扩展名匹配
				if (!bIsDirectory && FPaths::GetPathLeaf(FilenameOrDirectory) == InFileFullName)
				{
					OutFiles.Add(FilenameOrDirectory);
					return false;
				}
				return true; // 继续遍历
			});
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Directory does not exist: %s"), *Directory);
		return false;
	}
	return true;
}

FString GetUClassModuleDllPath(const UClass* InClass)
{
	if (!InClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid UClass pointer."));
		return "";
	}
	// InClass->GetOutermost()->
	// Get the module's name
	FString PackageName = InClass->GetOutermost()->GetName(); // Like [/Script/ModuleName]
	FString ModuleName = PackageName;
	ModuleName = ModuleName.Replace(TEXT("/Script/"), TEXT(""));
	ModuleName = ModuleName.Replace(TEXT("/Games/"), TEXT(""));

	// 获取模块文件路径
	if (!FModuleManager::Get().IsModuleLoaded(*ModuleName))
	{
		UE_LOG(LogTemp, Warning, TEXT("Module '%s' is not loaded!"), *ModuleName);
		return "";
	}
	FString ModuleDLL_FilePath = FModuleManager::Get().GetModuleFilename(*ModuleName);
	FString ModuleDLL_Dir = FPaths::GetPath(ModuleDLL_FilePath);
	return FPaths::ConvertRelativePathToFull(ModuleDLL_Dir);
}

FString MakeEngineSourcePath(const FString& InRelativePath)
{
	// EngineSourceDir = EngineSourceDir.Replace(TEXT("/Binaries/"), TEXT("/Source/"));
	// EngineSourceDir = EngineSourceDir.Replace(TEXT("/Plugins/"), TEXT("/Source/"));
	// EngineSourceDir = EngineSourceDir.Replace(TEXT("/Engine/"), TEXT("/Engine/Source/"));
	// const int SourceIdx = EngineSourceDir.Find(TEXT("/Source/"), ESearchCase::CaseSensitive, ESearchDir::FromEnd);
	// FString PlatformStr = EngineSourceDir.Right(EngineSourceDir.Len() - SourceIdx - 8);
	// EngineSourceDir = EngineSourceDir.Left(SourceIdx);
	FString EngineSourceDir = FPaths::EngineSourceDir();
	FString EngineSourcePath = EngineSourceDir / InRelativePath;
	return EngineSourcePath;
}

bool GetUClassSourceFiles(UClass* InClass, FString& OutHeaderPath, FString& OutSourcePath)
{
	if (!InClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid UClass pointer."));
		return false;
	}
FString ModuleName;

	// Get the module's name
	FString AbsoluteModuleDLL_Path = GetUClassModuleDllPath(InClass);
	// FString ModulePath = FModuleManager::Get().GetModuleFilename();

	// Get Engine Source Directory
	FString EngineSourceDir = FPaths::EngineSourceDir();
// FPaths::ProjectPluginsDir();
// FPaths::ProjectDir() / TEXT("Source");

	// Find the module's source folder path
	FString ModuleAbsolutePath = EngineSourceDir / ModuleName;
	if (!FPaths::DirectoryExists(ModuleAbsolutePath))
	{
		if (!FindFirstDirRecursively(EngineSourceDir, ModuleName, ModuleAbsolutePath))
		{
			return false;
		}
	}

	// Get Class Header's path relative to the module's source folder
	// Like [Public/SOME_FOLDERS/CLASS_NAME.h]
	FString ClassHeaderModuleRelativePath = InClass->GetMetaData(TEXT("ModuleRelativePath"));

	OutHeaderPath = ModuleAbsolutePath / ClassHeaderModuleRelativePath;
	OutSourcePath = OutHeaderPath;
	OutSourcePath.ReplaceInline(TEXT(".h"), TEXT(".cpp"));
	OutSourcePath.ReplaceInline(TEXT("Public"), TEXT("Private"));
	FString ClassSourceFileName = InClass->GetName() + TEXT(".cpp");
	FString ClassHeaderFileName = InClass->GetName() + TEXT(".h");

	if (!FPaths::FileExists(OutHeaderPath))
	{
		ensureMsgf(false, TEXT("Header file for class '%s' not found in module '%s'."), *InClass->GetName(),
		           *ModuleName);
		OutHeaderPath = "";
		OutSourcePath = "";
		return false;
	}

	if (!FPaths::FileExists(OutSourcePath))
	{
		TArray<FString> OutSourcePathList;
		FindFilesRecursively(EngineSourceDir, ClassSourceFileName, OutSourcePathList);
		if (OutSourcePathList.Num() == 1)
		{
			OutSourcePath = OutSourcePathList[0];
		}
		else if (OutSourcePathList.Num() > 1)
		{
			UE_LOG(LogTemp, Warning, TEXT("MULTIPLE Source file Found!!. ONLY use the first one."));
			for (const FString& SourcePath : OutSourcePathList)
			{
				UE_LOG(LogTemp, Warning, TEXT("\t\tSource file: %s"), *SourcePath);
			}
			OutSourcePath = OutSourcePathList[0];
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Source file for class '%s' not found in module '%s'."), *InClass->GetName(),
			       *ModuleName);
			OutSourcePath = "";
			return false;
		}
	}

	return true;
}

bool GetUClassSourceFilesByUsers(UClass* InClass, FString& OutHeaderPath, FString& OutSourcePath)
{
return true;
}


// bool FindModuleBasePath(const FString& InModuleName, FString& OutModuleBasePath)


/**
 * 尝试在项目源码、引擎源码、项目插件、引擎插件目录中查找模块的基本路径
 * @param InModuleName       模块名称（例如"MyModule"），要求与模块构建规则中定义的名称一致
 * @param OutModuleBasePath  如果找到，则返回模块基本路径（目录路径），该目录下通常包含模块的 Build.cs 文件和 Source 文件夹
 * @return                   如果找到则返回 true，否则返回 false
 */
bool FClassSourceSearcher::FindModuleBasePath(const FString& InModuleName, FString& OutModuleBasePath)
{
	if (ModulePathCacheMap.Contains(InModuleName))
	{
		OutModuleBasePath = ModulePathCacheMap[InModuleName];
		return true;
	}

	// 候选根目录数组
	TArray<FString> CandidateRoots;
#if ENGINE_MAJOR_VERSION >= 5
	FString UserProjectDir = FPaths::ProjectDir();
#else
	FString UserProjectDir = FPaths::GameDir();
#endif
	FString EngineRootDir = FPaths::ConvertRelativePathToFull(FPaths::EngineDir());

	// 1. 项目源码模块： [ProjectDir]/Source/[ModuleName]
	CandidateRoots.Add(FPaths::Combine(UserProjectDir, TEXT("Source")));
	// 2. 引擎源码模块： [EngineDir]/Source/[ModuleName]
	CandidateRoots.Add(FPaths::Combine(EngineRootDir, TEXT("Source")));
	// 3. 项目插件：遍历 [ProjectDir]/Plugins 下的所有插件
	CandidateRoots.Add(FPaths::Combine(UserProjectDir, TEXT("Plugins")));
	// 4. 引擎插件：遍历 [EngineDir]/Plugins 下的所有插件
	CandidateRoots.Add(FPaths::Combine(EngineRootDir, TEXT("Plugins")));

	for (FString CandidatePath : CandidateRoots)
	{
		IFileManager::Get().IterateDirectoryRecursively(
			*CandidatePath, [&](const TCHAR* FilenameOrDirectory, bool bIsDirectory) -> bool
			{
				if (bIsDirectory)
				{
					return true; // continue iterating
				}

				const FString FileName = FPaths::GetPathLeaf(FilenameOrDirectory);
				if (FileName.Contains(TEXT("Build.cs")))
				{
					FString ModuleBasePath = FPaths::GetPath(FilenameOrDirectory);
					FString ModuleName = FPaths::GetPathLeaf(ModuleBasePath);
					ModulePathCacheMap.Add(ModuleName, ModuleBasePath);
				}
				return true;
			});
	}

	if (!ModulePathCacheMap.Contains(InModuleName))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to find the base path for module '%s'."), *InModuleName);
		return false;
	}

	OutModuleBasePath = ModulePathCacheMap[InModuleName];
	return true;
}

bool FClassSourceSearcher::FindCPPSourceFile(const FString& InModuleName, const FString& InUClassModuleRelativePath)
{
	return true;
}

FString FClassSourceSearcher::GetUClassModuleName(UClass* InClass)
{
	if (!InClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid UClass pointer."));
		return "";
	}
	// 对于蓝图生成的类，沿着父类链查找第一个 Native 类
	UClass* NativeCppClass = InClass;
	while (NativeCppClass && !NativeCppClass->HasAnyClassFlags(CLASS_Native))
	{
		NativeCppClass = NativeCppClass->GetSuperClass();
	}
	if (!NativeCppClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to find the native class for UClass '%s'."), *InClass->GetName());
		return "";
	}

	FString PackageName = NativeCppClass->GetOutermost()->GetName();
	FString ModuleName = PackageName;
	ModuleName = ModuleName.Replace(TEXT("/Script/"), TEXT(""));
	ModuleName = ModuleName.Replace(TEXT("/Games/"), TEXT(""));
	return ModuleName;
}

bool FClassSourceSearcher::FindUClassSourceFiles(UClass* InClass, FString& OutHeaderPath, FString& OutSourcePath)
{
	FString ModuleName = GetUClassModuleName(InClass);
	FString ModuleBasePath;
	if (!FindModuleBasePath(ModuleName, ModuleBasePath))
	{
		return false;
	}

	if (ClassSourceFilesInfoCacheMap.Contains(InClass))
	{
		FClassSourceFilesInfo& ClassSourceFilesInfo = ClassSourceFilesInfoCacheMap[InClass];
		OutHeaderPath = ClassSourceFilesInfo.HeaderFilePath;
		OutSourcePath = ClassSourceFilesInfo.SourceFilePath;
		return true;
	}

	const FString ClassHeaderModuleRelativePath = InClass->GetMetaData(TEXT("ModuleRelativePath"));
	OutHeaderPath = ModuleBasePath / ClassHeaderModuleRelativePath;
	if (!FPaths::FileExists(OutHeaderPath))
	{
		ensureMsgf(false, TEXT("Header file for class '%s' not found in module '%s'."), *InClass->GetName(), *ModuleName);
		OutHeaderPath = "";
		OutSourcePath = "";
		return false;
	}

	OutSourcePath = OutHeaderPath;
	OutSourcePath.ReplaceInline(TEXT(".h"), TEXT(".cpp"));
	OutSourcePath.ReplaceInline(TEXT("Public"), TEXT("Private"));

	// The guessed source file path exist. We can return now.
	if (FPaths::FileExists(OutSourcePath))
	{
		ClassSourceFilesInfoCacheMap.Add(InClass, {InClass, OutHeaderPath, OutSourcePath});
		return true;
	}

	// The guessed source file path may not exist.
	// We need to search the source file in the module's source folder.
	TArray<FString> OutSourcePathList;
	FindFilesRecursively(ModuleBasePath, InClass->GetName() + TEXT(".cpp"), OutSourcePathList);
	if (OutSourcePathList.Num() == 1)
	{
		OutSourcePath = OutSourcePathList[0];
	}
	else if (OutSourcePathList.Num() > 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("MULTIPLE Source file Found!!. ONLY use the first one."));
		for (const FString& SourcePath : OutSourcePathList)
		{
			UE_LOG(LogTemp, Warning, TEXT("\t\tSource file: %s"), *SourcePath);
		}
		OutSourcePath = OutSourcePathList[0];
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Source file for class '%s' not found in module '%s'."), *InClass->GetName(),
		       *ModuleName);
		OutSourcePath = "";
		return false;
	}

	ClassSourceFilesInfoCacheMap.Add(InClass, {InClass, OutHeaderPath, OutSourcePath});
	return true;
}
