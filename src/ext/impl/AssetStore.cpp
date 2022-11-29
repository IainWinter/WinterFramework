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

    void WriteAssetPack(const std::string& filepath)
    {
        // write the map of assets in binary
        
        
        
        // open a binary serializer
    }

    void ReadAssetPack(const std::string& filepath)
    {
        
    }

    bool Has(const std::string& name)
    {
        AssetContext* ctx = GetContext();
        return ctx->m_loaded.find(name) != ctx->m_loaded.end();
    }

    void Free(const std::string& name)
    {
        AssetContext* ctx = GetContext();
        ctx->m_loaded.erase(name);
    }
}
