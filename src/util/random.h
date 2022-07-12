#pragma once

#include "Common.h"
#include <vector>
#include <utility>

template<typename _t>
size_t choosei(const std::vector<std::pair<_t, float>>& item_weights)
{
	float total_weight = 0.f;
	for (const auto& [item, weight] : item_weights)
	{
		total_weight += weight;
	}

	float pick = get_rand(1.f) * total_weight;
				
	float last_weight = 0.f;
	for (size_t i = 0; i < item_weights.size(); i++)
	{
		const auto& [item, weight] = item_weights.at(i);

		if (pick > last_weight && pick < weight + last_weight)
		{
			return i;
		}

		last_weight += weight;
	}

	return 0;
}

template<typename _t>
_t choose(const std::vector<std::pair<_t, float>>& item_weights)
{
	return item_weights[choosei<_t>(item_weights)].first;
}