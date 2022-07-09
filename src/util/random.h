#pragma once

#include "Common.h"
#include <vector>
#include <utility>

template<typename _t>
_t choose(const std::vector<std::pair<_t, float>>& item_weights)
{
	float total_weight = 0.f;
	for (const auto& [item, weight] : item_weights)
	{
		total_weight += weight;
	}

	float pick = get_rand(1.f) * total_weight;
				
	float last_weight = 0.f;
	for (const auto& [item, weight] : item_weights)
	{
		if (pick > last_weight && pick < weight + last_weight)
		{
			return item;
		}

		last_weight += weight;
	}

	return item_weights.back().first;
}