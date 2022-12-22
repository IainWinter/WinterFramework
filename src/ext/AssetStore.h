#pragma once

#include "Common.h"
#include "Log.h"
#include "ext/serial/serial.h"
#include "util/filesystem.h"

#include <unordered_map>
#include <vector>
#include <filesystem>

namespace Asset
{
	template<typename _t>
	struct AssetControlBlock
	{
		std::string name;
		r<_t> instance;

		int useCount = 0;
	};
}

template<typename _t>
struct AssetItem
{
private:
	// either point to the control block (Loaded)
	// or have a fake reference so this can be implicitly casted to a ref

	// If this is not expired, use data from here
	wr<Asset::AssetControlBlock<_t>> control;

	// Use data here if this is just a wrapper
	r<_t> pass;

public:
	AssetItem() = default;

	AssetItem(r<_t> asset)
		: pass      (asset) 
	{}

	AssetItem(r<Asset::AssetControlBlock<_t>> control)
		: control   (control)
	{
		inc_use_count(1);
	}

	// copy needs to change useCount

	AssetItem(const AssetItem& copy)
	{
		control = copy.control;
		pass = copy.pass;
		inc_use_count(1);
	}

	AssetItem(AssetItem&& move) noexcept
	{
		control = std::move(move.control);
		pass = std::move(move.pass);
	}

	AssetItem& operator=(const AssetItem& copy)
	{
		control = copy.control;
		pass = copy.pass;
		inc_use_count(1);
		return *this;
	}

	AssetItem& operator=(AssetItem&& move) noexcept
	{
		control = std::move(move.control);
		pass = std::move(move.pass);
		return *this;
	}

	~AssetItem()
	{
		inc_use_count(-1);
	}
	
	bool IsLoaded() const
	{
		return has_pass() || has_control();
	}

	const char* Name() const
	{
		if (has_control())
		{
			return control.lock()->name.c_str();
		}

		return has_pass() ? "Wrapped Asset" : "Unloaded Asset";
	}

	operator r<_t>() const { return has_control() ? control.lock()->instance : pass; }
	operator  bool() const { return IsLoaded(); }

	bool      operator!()  const { return !IsLoaded(); }

	      _t* operator->()       { return has_pass() ? pass.get() : control.lock()->instance.get(); }
	const _t* operator->() const { return has_pass() ? pass.get() : control.lock()->instance.get(); }
	
	      _t& operator* ()       { return *this->operator->(); }
	const _t& operator* () const { return *this->operator->(); }

private:
	bool has_pass()    const { return !!pass; }
	bool has_control() const { return !control.expired(); }

	void inc_use_count(int inc) { if (has_control()) control.lock()->useCount += inc; }
};

template<typename _t>
using a = AssetItem<_t>;

namespace Asset
{
	struct Loaded
	{
		r<AssetControlBlock<void>> control;

		meta::type* type;

		//meta::type* type;
		//r<void>     instance;

		// this should equal whoAreWe for assets
		// that are loaded in the context's heap
		int whoLoaded = 0;

		// master list of all references to this asset
		// this has to be kept to force an unload if requested
		// and to update the memory on a reallocation
		// 
		// this doesnt ork cus a<> are stack structs
		// 
		//std::vector<a<void>> references;

		// the ordering to load, unused

		// | Type         | Order |
		// ------------------------
		// | Texture      |   0   |
		// | TextureAtlas |   1   |

		//int dependancy = 0;

		Loaded(meta::type* type, const std::string& name, r<void> instance);
		meta::any any() const;

		//template<typename _t>
		//a<_t> NewReference();

		//template<typename _t>
		//a<_t> DeleteReference();
	};

	struct AssetContext
	{
		std::unordered_map<std::string, Loaded> loaded;
		int nextWhoAreWe = 0;

		template<typename _t>
		void RegisterAsset(const std::string& name, r<_t> instance);

		template<typename _t>
		a<_t> GetAsset(const std::string& name);
	};

	//
	//	Context
	//

	void CreateContext();
	void DestroyContext();
	void SetCurrentContext(AssetContext* context);
	AssetContext* GetContext();

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
	//	Getting avalible assets
	//

	template<typename _t>
	std::vector<a<_t>> GetAssetsOfType();

	//
	//	Dll fencing
	//

	void ReallocAssets();
	int WhoAreWe();

	//
	// Asset packing
	//

	void WriteAssetPack(const std::string& filepath);
	void  ReadAssetPack(const std::string& filepath);
}

//
//
//	impl
//
//

namespace Asset
{
	template<typename _t>
	void AssetContext::RegisterAsset(const std::string& name, r<_t> instance)
	{
		loaded.emplace(
			GetAssetName(name), 
			Loaded(meta::get_class<_t>(), name, instance)
		);
	}

	template<typename _t>
	a<_t> AssetContext::GetAsset(const std::string& name)
	{
		auto itr = loaded.find(GetAssetName(name));

		if (itr == loaded.end()) // return empty
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
			GetContext()->RegisterAsset<_t>(name, nullptr);
		}
		
		return Get<_t>(name);
	}

	template<typename _t>
	a<_t> Get(const std::string& name)
	{
		return GetContext()->GetAsset<_t>(name);
	}

	template<typename _t, typename... _a>
	a<_t> Make(const std::string& name, const _a&... args)
	{
		if (!Has(name))
		{
			GetContext()->RegisterAsset<_t>(name, mkr<_t>(args...));
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
			log_io("w~Failed to load asset from file: '%s'", filename.c_str());
			return a<_t>();
		}

		return Make<_t>(::_ar(filename), assetPath, args...);
	}

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