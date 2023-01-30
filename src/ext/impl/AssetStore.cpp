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
            RegisterLoaded(instance.name, instance.data.type(), instance.data.make_ref());
        }
    }
    
    void write_AssetContext(meta::serial_writer* serial, const AssetContext& instance)
    {
        std::vector<DiskAsset> assets;
        serial_dependency dep(meta::GetContext());

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
    wContextImpl(AssetContext);
	
    std::string GetAssetName(const std::string& name)
    {
        std::string n = name;
        std::replace(n.begin(), n.end(), '/', '\\');

        return n;
    }

    std::vector<a<void>> GetAssetsOfType(meta::id_type id)
    {
        std::vector<a<void>> assets;

        for (const auto& [name, asset] : GetContext()->loaded)
        {
            if (asset.type->info()->m_id == id)
            {
                assets.push_back(a<void>((r<AssetControlBlock<void>>&)asset.control));
            }
        }

        return assets;
    }

    void WriteAssetPack(const std::string& filepath)
    {
        log_io("Saving asset pack %s", filepath.c_str());

        std::ofstream out(filepath, std::fstream::binary);
        if (out.is_open()) bin_writer(out).write(*ctx.GetCurrent());
    }

    void ReadAssetPack(const std::string& filepath)
    {
        log_io("Loading asset pack %s", filepath.c_str());

        std::ifstream in(filepath, std::fstream::binary);
        if (in.is_open()) bin_reader(in).read(*ctx.GetCurrent());
    }

    void RegisterLoaded(const std::string& name, meta::type* type, r<void> instance)
    {
        ctx->loaded.emplace(GetAssetName(name), Loaded(type, name, instance));
    }

    bool Has(const std::string& name)
    {
        return ctx->loaded.find(GetAssetName(name)) != ctx->loaded.end();
    }

    void Free(const std::string& name)
    {
        auto itr = ctx->loaded.find(GetAssetName(name));

        if (itr != ctx->loaded.end())
        {
            ctx->loaded.erase(itr);
        }
    }
}

namespace Asset
{
    AssetContext::AssetContext()
    {
        meta::describe<AssetContext>()
            .custom_write(write_AssetContext)
            .custom_read ( read_AssetContext);

        meta::describe<DiskAsset>()
            .custom_write(write_DiskAsset)
            .custom_read ( read_DiskAsset);
    }

    void AssetContext::Realloc(int location)
    {
        // Not sure if this truly works
        // it seems like it should, but I wonder if the values in ctx->loaded are still
        // invalid
        
        // need a way to test this better, it seems like it may link the dll
        // to the same place everytime which may hide some bugs

        for (auto& [name, loaded] : ctx->loaded)
        {
            if (location == loaded.whoLoaded) // dont worry about memory allocated by us
            {
                continue;
            }

            // reallocation will change ownership
            loaded.whoLoaded = location;
            
            // create new memory in this module & call copy constructor
            meta::any replacement = loaded.type->construct();
            replacement.copy_in(loaded.control->instance.get());

            // replace instance with new memory
            loaded.control->instance = replacement.make_ref();
        }
    }

    Loaded::Loaded(meta::type* type_, const std::string& name, r<void> instance)
    {
        type = type_;

        control = mkr<AssetControlBlock<void>>();
        control->name = name;
        control->instance = instance;

        whoLoaded = GetContextLocation();
    }

    meta::any Loaded::any() const
    {
        return meta::any(type, control->instance.get());
    };
}