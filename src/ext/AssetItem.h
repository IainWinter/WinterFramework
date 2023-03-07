#pragma once

#include "util/ref.h"
#include <string>

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
		: pass (asset)
	{}

	AssetItem(r<Asset::AssetControlBlock<_t>> control)
		: control (control)
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

	AssetItem<void>& AsGeneric()
	{
		// unsafe cast, but this works for these classes in particular
		return *(AssetItem<void>*)this;
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

	      r<_t> ref()       { return has_control() ? control.lock()->instance : pass; }
	const r<_t> ref() const { return has_control() ? control.lock()->instance : pass; }

	      _t* ptr()       { return ref().get(); }
	const _t* ptr() const { return ref().get(); }

	operator const r<_t>() const { return ref(); }
	operator r<_t>()             { return ref(); }
	
	      _t* operator->()       { return ptr(); }
	const _t* operator->() const { return ptr(); }

	operator bool() const { return IsLoaded(); }
	bool operator!() const { return !IsLoaded(); }

private:
	bool has_pass()    const { return !!pass; }
	bool has_control() const { return !control.expired(); }

	void inc_use_count(int inc) { if (has_control()) control.lock()->useCount += inc; }
};

template<typename _t>
using a = AssetItem<_t>;