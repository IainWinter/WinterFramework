#include "ext/AssetStore.h"
#include "ext/serial/serial_bin.h"
#include <fstream>

namespace Asset
{
    void write_asset_pack(meta::serial_writer* serial, const AssetContext& instance)
    {
        std::vector<std::string> names;
        std::vector<meta::any> data;

        for (auto [_, loaded] : instance.loaded)
        {
            names.push_back(loaded.name);
            data.push_back(meta::any(loaded.type, loaded.original.get()));
        }

        serial->pseudo().begin<AssetContext>()
            .member("names", names)
            .member("data", data)
        .end();
    }

    void read_asset_pack(meta::serial_reader* serial, AssetContext& instance)
    {
        std::vector<std::string> names;
        std::vector<meta::any> data;

        serial->pseudo().begin<AssetContext>()
            .member("names", names)
            .member("data", data)
        .end();

        for (size_t i = 0; i < names.size(); i++)
        {
            const std::string& name = names.at(i);
            const meta::any& any = data.at(i);

            // this swaps the data inside the asset if its already loaded
            // this should allow hot loading on all assets

            if (instance.loaded.count(name) > 0)
            {
                any.copy_to(instance.loaded.at(name).instance.get());
            }

            else
            {
                AssetContext::Loaded l;
                l.name = names.at(i);
                l.instance = any.make_ref();
                l.original = any.make_ref();
                l.type = any.type();
                
                instance.loaded.emplace(name, l);
            }
        }

        //for (auto [_, loaded] : instance.loaded)
        //{
        //    names.push_back(loaded.name);
        //    data.push_back(meta::any(loaded.type, loaded.instance.get()));
        //}

        //serial->pseudo().begin<AssetContext>()
        //    .member("names", names)
        //    .member("data", data)
        //.write();
    }
}

namespace Asset
{
	AssetContext* ctx;

	void CreateContext()
	{
        DestroyContext();
		ctx = new AssetContext();

        meta::describe<AssetContext>()
            .custom_write(write_asset_pack)
            .custom_read ( read_asset_pack);
	}

	void DestroyContext()
	{
		delete ctx;
	}

	void SetCurrentContext(AssetContext* context)
	{
        DestroyContext();
		ctx = context;
	}

	AssetContext* GetContext()
	{
		return ctx;
	}

    void WriteAssetPack(const std::string& filepath)
    {
        std::ofstream out(filepath);
        bin_writer(out).write(*GetContext());



        // write the map of assets in binary
        
        //ctx->loaded
        
        // open a binary serializer
    }

    void ReadAssetPack(const std::string& filepath)
    {
        std::ifstream in(filepath);
        bin_reader(in).read(*GetContext());
    }

    bool Has(const std::string& name)
    {
        AssetContext* ctx = GetContext();
        return ctx->loaded.find(name) != ctx->loaded.end();
    }

    void Free(const std::string& name)
    {
        AssetContext* ctx = GetContext();
        ctx->loaded.erase(name);
    }
}
