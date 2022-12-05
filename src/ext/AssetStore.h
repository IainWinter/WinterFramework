#pragma once

#include "Common.h"
#include "ext/serial/serial.h"

#include <unordered_map>
#include <vector>

template<typename _t>
struct AssetItem
{
	const char* name = nullptr;
	r<_t> asset;

	AssetItem() : name(nullptr), asset(nullptr) {}
	AssetItem(r<_t> asset) : name(nullptr), asset(asset) {}
	AssetItem(const char* name, r<_t> asset) : name(name), asset(asset) {}

	operator r<_t>() const { return asset; }
	operator  bool() const { return !!asset; }

	      _t* operator->()       { return asset.get(); }
	      _t& operator* ()       { return *asset; }
	const _t* operator->() const { return asset.get(); }
	const _t& operator* () const { return *asset; }
	bool      operator! () const { return !asset; }
};

// rename like r<>
template<typename _t>
using a = AssetItem<_t>;

namespace Asset
{
	struct AssetContext
	{
		struct Loaded
		{
			std::string name;
			meta::type* type;
			r<void>     instance;
			r<void>     original; // this is to pack into an asset pack, it is needed because when sending
								  // some assets to the device, they are freeed from the host.

			template<typename _t>
			a<_t> GetAsset() { return a<_t>(name.c_str(), (r<_t>&)instance); }
		};
    
		std::unordered_map<std::string, Loaded> loaded;

		template<typename _t>
		void RegisterAsset(const std::string& name, r<_t> instance)
		{
			Loaded l;
			l.name = name;
			l.type = meta::get_class<_t>();
			l.instance = instance;
			l.original = mkr<_t>(*instance); // make a copy

			loaded.emplace(name, l).first;
		}
	};

	void CreateContext();
	void DestroyContext();
	void SetCurrentContext(AssetContext* context);
	AssetContext* GetContext();

    bool Has (const std::string& name);
    void Free(const std::string& name);

	template<typename _t>
	a<_t> Get(const std::string& name)
	{
		return GetContext()->loaded.at(name).GetAsset<_t>();
	}

	template<typename _t, typename... _a>
	a<_t> Load(const std::string& name, const _a&... args)
	{
		if (!Has(name))
		{
			GetContext()->RegisterAsset<_t>(name, mkr<_t>(args...));
		}

		return Get<_t>(name);
	}

	// This loads an asset by passing the filename as the first arg
	// in the constructor
	template<typename _t, typename... _a>
	a<_t> LoadFromFile(const std::string& filename, const _a&... args)
	{
		return Load<_t, _a...>(filename, filename, args...);
	}

	// Find the asset and free it
	template<typename _t>
	void Free(const r<_t>& asset)
	{
		AssetContext* ctx = GetContext();

		for (auto [name, loaded] : ctx->loaded)
		{
			if (asset == loaded) // compare pointers
			{
				Free(name);
				return;
			}
		}
	}

	// Asset packing

	void WriteAssetPack(const std::string& filepath);
	void  ReadAssetPack(const std::string& filepath);
}

namespace meta
{
	template<typename _t>
	void serial_write(serial_writer* serial, const a<_t>& instance)
	{
		const char* protectedName = instance.name ? instance.name : "";
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
