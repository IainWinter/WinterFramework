#pragma once

#include "Rendering.h"
#include "Common.h"
#include <vector>
#include <tuple>
#include <unordered_set>

#include "Sand/SandComponents.h"

struct Islands
{
	using Group = std::vector<std::vector<int>>;

	Group coreIslands;
	Group otherIslands;

	size_t Count() const
	{
		return coreIslands.size() + otherIslands.size();
	}
};

template<typename _t>
std::tuple<int, int, int, int> GetBoundingBoxOfIsland(const _t& island, int width)
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
// a health pixel has an alpha not equal to 0 or 255, 
// could make a third texture for this, but seems unnessesary, no sprites have opacity as of now...
CorePixels GetCorePixels(const r<Texture>& texture);

Islands GetIslands(const SandSprite& sprite);

//mat2 GetCoord(const Transform2D& transform, int cellsPerMeter);
//vec2 GetLocalPosOfPixel(mat2 coord, int index, const r<Texture>& texture);
