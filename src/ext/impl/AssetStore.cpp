#include "ext/AssetStore.h"

namespace Asset
{
	AssetContext* ctx;

	void CreateContext()
	{
		ctx = new AssetContext();
	}

	void DestroyContext()
	{
		delete ctx;
	}

	void SetCurrentContext(AssetContext* context)
	{
		ctx = context;
	}

	AssetContext* GetContext()
	{
		return ctx;
	}
}