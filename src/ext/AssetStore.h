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

struct AssetContext
{
	std::unordered_map<std::string, r<void>> m_loaded;
};

namespace Asset
{
	void CreateContext();
	void DestroyContext();
	void SetCurrentContext(AssetContext* context);
	AssetContext* GetContext();

	inline bool Has(const std::string& name)
	{
		AssetContext* ctx = GetContext();
		return ctx->m_loaded.find(name) != ctx->m_loaded.end();
	}

	template<typename _t>
	a<_t> Get(const std::string& name)
	{
		AssetContext* ctx = GetContext();

		auto itr = ctx->m_loaded.find(name);
		return a<_t>(itr->first.c_str(), (r<_t>&)itr->second);
	}

	template<typename _t, typename... _a>
	a<_t> Load(const std::string& name, const _a&... args)
	{
		AssetContext* ctx = GetContext();

		auto itr = ctx->m_loaded.find(name);
		if (itr == ctx->m_loaded.end())
		{
			itr = ctx->m_loaded.emplace(name, mkr<_t>(args...)).first;
		}

		return a<_t>(itr->first.c_str(), (r<_t>&)itr->second);
	}

	inline void Free(const std::string& name)
	{
		AssetContext* ctx = GetContext();
		ctx->m_loaded.erase(name);
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

		for (auto [name, loaded] : ctx->m_loaded)
		{
			if (asset == loaded) // compare pointers
			{
				Free(name);
				return;
			}
		}
	}
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

		// asset must be loaded
		// this will be done through an asset pack loader
		if (Asset::Has(name))
		{
			instance = Asset::Get<_t>(name);
		}
	}
}
