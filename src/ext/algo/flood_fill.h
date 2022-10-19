#pragma once

#include <vector>
#include <unordered_set>
#include <functional>

enum class ff_cell : char
{
	VISITED  = -1,

	EMPTY    = (char)false,
	FILLED   = (char)true,

	MASK_1   = 2,
	MASK_2,
	MASK_3,
	MASK_4,
	MASK_5,
};

template<typename _t>
std::vector<ff_cell> flood_fill_get_states_from_array(_t* arr, size_t length, std::function<ff_cell(const _t&)> test)
{
	std::vector<ff_cell> states;
	states.reserve(length);
	for (int i = 0; i < length; i++)
	{
		states.push_back(test(arr[i]));
	}
	return states;
}

struct flood_fill_result
{
    std::vector<int> index;
    std::unordered_set<ff_cell> types;
};

inline
flood_fill_result flood_fill(int seed, int size_x, int size_y, std::vector<ff_cell>& cells, ff_cell mask = ff_cell::FILLED, ff_cell set_filled = ff_cell::VISITED, bool diagonals = false)
{
    assert(mask != set_filled && "Mask and filled must be different");
    
    flood_fill_result result;
    
	std::vector<int> queue;
	queue.push_back(seed);

	while (queue.size() > 0)
	{
		int index = queue.back(); queue.pop_back();

		if (   index < 0
			|| index >= size_x * size_y)
		{
			continue;
		}

		if (cells[index] >= mask)
		{
            result.index.push_back(index);
            result.types.insert(cells[index]);
            
            cells[index] = set_filled;

			bool natLeft  = index % size_x != 0;
			bool natRight = index % size_x != size_x - 1;

			if (natRight) queue.push_back(index + 1);
			if (natLeft)  queue.push_back(index - 1);
			              queue.push_back(index + size_x);
			              queue.push_back(index - size_x);

			if (diagonals)
			{
				if (natRight) queue.push_back(index + 1 + size_x);
				if (natRight) queue.push_back(index + 1 - size_x);
				if (natLeft)  queue.push_back(index - 1 + size_x);
				if (natLeft)  queue.push_back(index - 1 - size_x);
			}
		}
	}

	return result;
}
