#include "Sand/SandHelpers.h"
#include "ext/algo/flood_fill.h"

CorePixels GetCorePixels(const r<Texture>& texture)
{
	CorePixels pixels;

	u32* itr = (u32*)texture->Pixels();
	u32* end = itr + texture->Length();

	for (int i = 0; itr != end; itr++, i++)
	{
		u8 alpha = Color(*itr).a;

		if (alpha > 0)
		{
			pixels.Add(i, alpha < 255);
		}
	}

	return pixels;
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

	for (int seed : sprite.initalCore)
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

//mat2 GetCoord(const Transform2D& transform, int cellsPerMeter)
//{
//	float s = 1.f / cellsPerMeter;
//	float r = transform.rotation;
//	return mat2(cos(r), sin(r), -sin(r), cos(r))
//		 * mat2(2*s, 0, 0, 2*s);
//}
//
//vec2 GetWorldPosOfPixel(mat2 coord, int index, const r<Texture>& texture)
//{
//	auto [x, y] = get_xy(index, texture->Width());
//	vec2 pos = coord * (vec2(x, y) - texture->Dimensions() / 2.f + vec2(.5f, .5f));
//	return vec2();
//}
