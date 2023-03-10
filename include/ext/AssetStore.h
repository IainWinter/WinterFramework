#pragma once

#include "Common.h"
#include "Log.h"
#include "ext/serial/serial.h"
#include "ext/AssetItem.h"

#include "util/filesystem.h"
#include "util/context.h"

#include <unordered_map>
#include <vector>
#include <filesystem>

namespace Asset
{
	struct Loaded
	{
		r<AssetControlBlock<void>> control;

		meta::type* type;

		// this should equal whoAreWe for assets
		// that are loaded in the context's heap
		int whoLoaded = 0;

		Loaded(meta::type* type, const std::string& name, r<void> instance);
		meta::any any() const;
	};

	struct AssetContext : wContext
	{
		std::unordered_map<std::string, Loaded> loaded;

		AssetContext();

		void Realloc(int location) override;
	};

	//
	//	Context
	//

	wContextDecl(AssetContext);

	//
	//  Asset name
	//

	std::string GetAssetName(const std::string& name);

	//
	// Loading / freeing assets
	//

    bool Has (const std::string& name);
    void Free(const std::string& name);

	template<typename _t> a<_t> Dummy(const std::string& name);
	template<typename _t> a<_t>   Get(const std::string& name);
	template<typename _t> void   Free(const r<_t>& asset);

	template<typename _t, typename... _a> a<_t> Make        (const std::string& name,     const _a&... args);
	template<typename _t, typename... _a> a<_t> LoadFromFile(const std::string& filename, const _a&... args);

	//
	//	Getting available assets
	//

	template<typename _t>
	std::vector<a<_t>> GetAssetsOfType();

	std::vector<a<void>> GetAssetsOfType(meta::id_type id);

	//
	// Asset packing
	//

	void WriteAssetPack(const std::string& filepath);
	void  ReadAssetPack(const std::string& filepath);

	void RegisterLoaded(const std::string& name, meta::type* type, r<void> instance);
}

//
//
//	impl
//
//

namespace Asset
{
	template<typename _t>
	void RegisterAsset(const std::string& name, r<_t> instance)
	{
		RegisterLoaded(name, meta::get_class<_t>(), instance);
	}

	template<typename _t>
	a<_t> GetAsset(const std::string& name)
	{
		const AssetContext* ctx = GetContext();

		auto itr = ctx->loaded.find(GetAssetName(name));

		if (itr == ctx->loaded.end()) // return empty
		{
			return a<_t>();
		}

		return a<_t>( (r<AssetControlBlock<_t>>&)itr->second.control );
	}

	template<typename _t>
	a<_t> Dummy(const std::string& name)
	{
		if (!Has(name))
		{
			RegisterAsset<_t>(name, nullptr);
		}
		
		return Get<_t>(name);
	}

	template<typename _t>
	a<_t> Get(const std::string& name)
	{
		return GetAsset<_t>(name);
	}

	template<typename _t, typename... _a>
	a<_t> Make(const std::string& name, const _a&... args)
	{
		if (!Has(name))
		{
			RegisterAsset<_t>(name, mkr<_t>(args...));
		}

		return Get<_t>(name);
	}

	template<typename _t, typename... _a>
	a<_t> LoadFromFile(const std::string& filename, const _a&... args)
	{
		if (Has(filename))
		{
			return Get<_t>(filename);
		}

		std::string assetPath = ::_a(filename);

		if (!std::filesystem::exists(std::filesystem::path(assetPath)))
		{
			log_io("w~Failed to load asset from file: '%s'", assetPath.c_str());
			return a<_t>();
		}

		return Make<_t>(::_ar(filename), assetPath, args...);
	}

	template<typename _t>
	void Free(const r<_t>& asset)
	{
		for (auto [name, loaded] : GetContext()->loaded)
		{
			if (asset == loaded) // compare pointers
			{
				Free(name);
				return;
			}
		}
	}

	template<typename _t>
	std::vector<a<_t>> GetAssetsOfType()
	{
		std::vector<a<_t>> assets;

		for (const auto& [name, asset] : GetContext()->loaded)
		{
			if (asset.type->info()->m_id == meta::id<_t>())
			{
				assets.push_back(a<_t>( (r<AssetControlBlock<_t>>&)asset.control) );
			}
		}

		return assets;
	}
}

namespace meta
{
	template<typename _t>
	void describer(type* type, tag<a<_t>>)
	{
		type->set_prop("generic_write", true);
		type->set_prop("generic_read", true);
		type->set_prop("is_asset", true);
		type->set_prop("inner_type", id<_t>());
	}

	template<typename _t>
	void serial_write(serial_writer* serial, const a<_t>& instance)
	{
		const char* protectedName = instance.Name() ? instance.Name() : "";
		serial->write_string(protectedName, strlen(protectedName));
	}

	template<typename _t>
	void serial_read(serial_reader* serial, a<_t>& instance)
	{
		std::string name;
		
		name.resize(serial->read_length());
		serial->read_string(name.data(), name.size());

		if (!name.empty())
		{
			instance = Asset::LoadFromFile<_t>(name);
		}
	}
}