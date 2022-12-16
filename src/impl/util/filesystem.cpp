#include "util/filesystem.h"

namespace File
{
	FileContext* ctx;

	void CreateContext()
	{
		DestroyContext();
		ctx = new FileContext();
	}

	void DestroyContext()
	{
		delete ctx;
	}

	void SetCurrentContext(FileContext* context)
	{
		ctx = context;
	}

	FileContext* GetContext()
	{
		return ctx;
	}

	void PushCurrentPath(const std::string& path)
	{
		try
		{
			std::filesystem::path old (std::filesystem::current_path());
			std::filesystem::path next(path);

			std::filesystem::current_path(next);

			if (ctx->currentPathStack.size() == 0)
			{
				ctx->currentPathStack.push(old);
			}

			ctx->currentPathStack.push(next);
		}

		catch (std::exception e)
		{
			// soft error
		}
	}

	void PopCurrentPath()
	{
		if (ctx->currentPathStack.size() == 1) // ext on no previous paths
		{
			return;
		}

		try 
		{
			ctx->currentPathStack.pop();

			std::filesystem::path next = ctx->currentPathStack.top();
			std::filesystem::current_path(next);
		}

		catch (std::exception e)
		{
			// soft error
		}
	}

	void SetAssetPath(const std::string& path)
	{
		ctx->assetRootPath = std::filesystem::absolute(path);
	}

	std::string GetAssetPath()
	{
		return ctx->assetRootPath.string();
	}
}

std::string _a(const std::string& assetPath)
{
	std::filesystem::path path(assetPath);

	if (path.is_absolute())
	{
		return assetPath;
	}

	return (File::ctx->assetRootPath / assetPath).string();
}

std::string _ar(const std::string& fullpath)
{
	std::string root = File::GetAssetPath();

	size_t rootLength = root.size() + 1; // +1 for '/'
	size_t rootIndex  = fullpath.find(root);

	if (fullpath.size() <= rootLength || rootIndex == std::string::npos)
	{
		return fullpath;
	}

	return fullpath.substr(rootLength);
}