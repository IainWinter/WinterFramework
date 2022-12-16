#include "ext/AssetStore.h"
#include "ext/serial/serial_bin.h"
#include "ext/serial/serial_dependency.h"
#include <fstream>
#include <algorithm>

namespace Asset
{
    struct DiskAsset
    {
        int dependency;
        std::string name;
        meta::any data;
    };

    void write_DiskAsset(meta::serial_writer* serial, const DiskAsset& instance)
    {
        serial->pseudo().begin<DiskAsset>()
            .member("dependency", instance.dependency)
            .member("name", instance.name)
            .member("data", instance.data)
            .end();
    }

    void read_DiskAsset(meta::serial_reader* serial, DiskAsset& instance)
    {
        serial->pseudo().begin<DiskAsset>()
            .member("dependency", instance.dependency)
            .member("name", instance.name)
            .member("data", instance.data)
            .end();

        auto& loaded = Asset::GetContext()->loaded;

        if (loaded.count(instance.name) > 0)
        {
            // might be wrong?
            instance.data.copy_to(loaded.at(instance.name).control->instance.get());
        }

        else
        {
            // RegisterAsset basically

            loaded.emplace(
                instance.name, 
                Loaded(instance.data.type(), instance.name, instance.data.make_ref())
            );
        }
    }
    
    void write_AssetContext(meta::serial_writer* serial, const AssetContext& instance)
    {
        std::vector<DiskAsset> assets;
        serial_dependency dep(meta::get_context());

        for (const auto& [name, loaded] : instance.loaded)
        {
            DiskAsset disk;
            disk.name = name;
            disk.data = loaded.any();
            disk.dependency = dep.order(loaded.type->get_type());

            assets.push_back(disk);
        }

        std::sort(assets.begin(), assets.end(), 
            [](const DiskAsset& a, const DiskAsset& b)
            {
                return a.dependency < b.dependency;
            }
        );

        serial->pseudo().begin<AssetContext>()
            .member("assets", assets)
        .end();
    }

    void read_AssetContext(meta::serial_reader* serial, AssetContext& instance)
    {
        std::vector<DiskAsset> assets;

        serial->pseudo().begin<AssetContext>()
            .member("assets", assets)
        .end();
    }
}

namespace Asset
{
	AssetContext* ctx;

    // this is an id that counts up everytime we set the curernt context
    // gets used to identify the current dll fense
    int whoAreWe = 0;

    std::string GetAssetName(const std::string& name)
    {
        std::string n = name;
        std::replace(n.begin(), n.end(), '/', '\\');

        return n;
    }

    void CreateContext()
	{
        DestroyContext();
        SetCurrentContext(new AssetContext());

        meta::describe<AssetContext>()
            .custom_write(write_AssetContext)
            .custom_read ( read_AssetContext);

        meta::describe<DiskAsset>()
            .custom_write(write_DiskAsset)
            .custom_read ( read_DiskAsset);
	}

	void DestroyContext()
	{
		delete ctx;
	}

	void SetCurrentContext(AssetContext* context)
	{
		ctx = context;

        whoAreWe = ctx->nextWhoAreWe;
        ctx->nextWhoAreWe += 1;
	}

	AssetContext* GetContext()
	{
		return ctx;
	}

    void WriteAssetPack(const std::string& filepath)
    {
        log_io("Saving asset pack %s", filepath.c_str());

        std::ofstream out(filepath, std::fstream::binary);
        if (out.is_open()) bin_writer(out).write(*GetContext());
    }

    void ReadAssetPack(const std::string& filepath)
    {
        log_io("Loading asset pack %s", filepath.c_str());

        std::ifstream in(filepath, std::fstream::binary);
        if (in.is_open()) bin_reader(in).read(*GetContext());
    }

    void ReallocAssets()
    {
        // Not sure if this truly works
        // it seems like it should, but I wonder if the values in ctx->loaded are still
        // invalid
        
        // need a way to test this better, it seems like it may link the dll
        // to the same place everytime which may hide some bugs

        for (auto& [name, loaded] : ctx->loaded)
        {
            if (whoAreWe == loaded.whoLoaded) // dont worry about memory allocated by us
            {
                continue;
            }

            // reallocation will change ownership
            loaded.whoLoaded = whoAreWe;
            
            // create new memory in this module & call copy constructor
            meta::any replacement = loaded.type->construct();
            replacement.copy_in(loaded.control->instance.get());

            // replace instance with new memory
            loaded.control->instance = replacement.make_ref();
        }
    }

    int WhoAreWe()
    {
        return whoAreWe;
    }

    bool Has(const std::string& name)
    {
        AssetContext* ctx = GetContext();
        return ctx->loaded.find(GetAssetName(name)) != ctx->loaded.end();
    }

    void Free(const std::string& name)
    {
        AssetContext* ctx = GetContext();

        auto itr = ctx->loaded.find(GetAssetName(name));

        if (itr != ctx->loaded.end())
        {
            ctx->loaded.erase(itr);
        }
    }
}

namespace Asset
{
    Loaded::Loaded(meta::type* type_, const std::string& name, r<void> instance)
    {
        type = type_;

        control = mkr<AssetControlBlock<void>>();
        control->name = name;
        control->instance = instance;

        whoLoaded = WhoAreWe();
    }

    meta::any Loaded::any() const
    {
        return meta::any(type, control->instance.get());
    };
}