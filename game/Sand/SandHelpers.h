#pragma once

#include "Rendering.h"
#include "Common.h"
#include <vector>
#include <tuple>

inline std::tuple<int, int, int, int> GetBoundingBoxOfIsland(const std::vector<int>& island, int width)
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

// returns only health pixels, or all pixels if there are none
// a health pixel has an alpha not equal to 0 or 255, could make a third texture for this, but seems unnessesary, no sprites have opacity as of now...

struct CorePixels
{
	std::vector<int> core;
	std::vector<int> all;
	bool hasCore;
	bool hasAny;
};

inline CorePixels GetCorePixels(const r<Texture>& texture)
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