#pragma once

#include <vector>
#include <functional>

enum class flood_fill_cell_state : char
{
	EMPTY,
	FILLED,
	VISITED
};

template<typename _t>
std::vector<flood_fill_cell_state> flood_fill_get_states_from_array(_t* arr, size_t length, std::function<bool(const _t&)> test)
{
	std::vector<flood_fill_cell_state> states;
	states.reserve(length);
	for (int i = 0; i < length; i++)
	{
		states.push_back(test(arr[i]) ? flood_fill_cell_state::FILLED : flood_fill_cell_state::EMPTY);
	}
	return states;
}

inline
std::vector<int> flood_fill(int seed, int size_x, int size_y, std::vector<flood_fill_cell_state>& cells, bool diagonals = false)
{
	std::vector<int> island;

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

		if (cells[index] == flood_fill_cell_state::FILLED)
		{
			cells[index] = flood_fill_cell_state::VISITED;
			
			island.push_back(index);

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

	return island;
}