#include "Sand/SandHelpers.h"
#include "ext/flood_fill.h"

std::tuple<int, int, int, int> GetBoundingBoxOfIsland(const std::vector<int>& island, int width)
{
	int minX =  INT_MAX;
	int minY =  INT_MAX;
	int maxX = -INT_MAX;
	int maxY = -INT_MAX;

	for (const int& index : island)
	{
		auto [x, y] = get_xy(index, width);
		if (x < minX) minX = x;
		if (y < minY) minY = y;
		if (x > maxX) maxX = x;
		if (y > maxY) maxY = y;
	}

	return { minX, minY, maxX, maxY };
}

CorePixels GetCorePixels(const r<Texture>& texture)
{
	std::vector<int> filled, core;

	u32* itr = (u32*)texture->Pixels();
	u32* end = itr + texture->Length();

	for (int i = 0; itr != end; itr++, i++)
	{
		u8 alpha = Color(*itr).a;

		if (alpha > 0) filled.push_back(i);
		if (alpha > 0 && alpha < 255) core.push_back(i);
	}

	return { core, filled, core.size() > 0, filled.size() > 0 };
}

// finding islands

std::vector<flood_fill_cell_state> GetSpriteStates(const r<Texture>& mask)
{
	return flood_fill_get_states_from_array<u32>(
		(u32*)mask->Pixels(), mask->Length(), [](const u32& x) { return Color(x).a > 0; }
	);
}

void AddSingleIsland(int seed, const r<Texture>& mask, bool diags, std::vector<flood_fill_cell_state>& state, std::vector<std::vector<int>>& islands)
{
	auto island = flood_fill(seed, mask->Width(), mask->Height(), state, diags);
	if (island.size() > 0)
	{
		islands.emplace_back(std::move(island));
	}
}

Islands GetIslands(const SandSprite& sprite)
{
	Islands islands;

	std::vector<flood_fill_cell_state> state = GetSpriteStates(sprite.colliderMask);

	for (int seed : sprite.core)
	{
		AddSingleIsland(seed, sprite.colliderMask, sprite.isCircle, state, islands.coreIslands);
	}

	for (int i = 0; i < state.size(); i++)
	{
		if (state.at(i) == flood_fill_cell_state::FILLED)
		{
			AddSingleIsland(i, sprite.colliderMask, sprite.isCircle, state, islands.otherIslands);
		}
	}

	return islands;
}

