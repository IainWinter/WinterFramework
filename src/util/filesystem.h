#pragma once

#include <stack>
#include <filesystem>

namespace File
{
	struct FileContext
	{
		std::stack<std::filesystem::path> currentPathStack;
		std::filesystem::path assetRootPath;
	};

	void CreateContext();
	void DestroyContext();
	void SetCurrentContext(FileContext* context);
	FileContext* GetContext();

	void PushCurrentPath(const std::string& path);
	void  PopCurrentPath();

	void SetAssetPath(const std::string& path);
	std::string GetAssetPath();
}

// If assetPath is an absolute path, don’t touch it
// but if it's a relative path, return an absolute path rooted at File::GetAssetPath()
//
std::string _a(const std::string& assetPath);

///  Convert a fullpath to a relitive path into the assets folder, if possible
///
std::string _ar(const std::string& fullpath);