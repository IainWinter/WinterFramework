#pragma once

#include <functional>
#include <vector>

template<typename _t>
struct callback
{
	std::vector<std::function<_t>> functions;

	callback& operator +=(const std::function<_t>& function)
	{
		functions.push_back(function);
		return *this;
	}

	callback& operator -=(const std::function<_t>& function)
	{
		auto itr = std:find(functions.begin(), functions.end(), function);
		functions.erase(itr);
		return *this;
	}

	template<typename... _args>
	void operator()(const _args&... args)
	{
		for (const auto& function : functions)
		{
			function(args...);
		}
	}
};