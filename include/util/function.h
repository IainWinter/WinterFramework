#pragma once

#include <functional>
#include <vector>

template<typename _t>
struct comparable_func
{
    std::function<_t> function;

	comparable_func() : function() {}
	comparable_func(const std::function<_t>& function) : function(function) {}

	bool operator==(const comparable_func& other) const { return function.template target<_t>() == other.function.template target<_t>(); }
	bool operator!=(const comparable_func& other) const { return !operator==(other); };

	template<typename... _args>
	void operator()(const _args&... args) const {
		function(args...);
	}
};

template<typename _t>
struct func
{
	using func_type = comparable_func<_t>;

	std::vector<func_type> functions;

	func& add(const func_type& function) { return *this += function; }
	func& del(const func_type& function) { return *this -= function; }

	func& operator+=(const func_type& function)
	{
		functions.emplace_back(function);
		return *this;
	}

	func& operator-=(const func_type& function)
	{
		auto itr = std::find(functions.begin(), functions.end(), function);
		functions.erase(itr);
		return *this;
	}

	template<typename... _args>
	void operator()(const _args&... args) const
	{
		for (const auto& function : functions)
			function(args...);
	}
};
