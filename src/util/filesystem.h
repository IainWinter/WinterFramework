#pragma once

#include "util/context.h"

#include <stack>
#include <filesystem>

namespace File
{
	struct FileContext : wContext
	{
		std::stack<std::filesystem::path> currentPathStack;
		std::filesystem::path assetRootPath;
	};

	wContextDecl(FileContext);

	void PushCurrentPath(const std::string& path);
	void  PopCurrentPath(int number = 1);

	void SetAssetPath(const std::string& path);
	std::string GetAssetPath();
}

#define _A(path) ASSET_ROOT_PATH "/" path

// If assetPath is an absolute path, don’t touch it
// but if it's a relative path, return an absolute path rooted at File::GetAssetPath()
//
std::string _a(const std::string& assetPath);

///  Convert a fullpath to a relative path into the assets folder, if possible
///
std::string _ar(const std::string& fullpath);