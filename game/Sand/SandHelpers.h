#pragma once

#include "Rendering.h"
#include "Common.h"
#include <vector>
#include <tuple>

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

std::tuple<int, int, int, int> GetBoundingBoxOfIsland(const std::vector<int>& island, int width);

// returns only health pixels, or all pixels if there are none
// a health pixel has an alpha not equal to 0 or 255, 
// could make a third texture for this, but seems unnessesary, no sprites have opacity as of now...
CorePixels GetCorePixels(const r<Texture>& texture);

Islands GetIslands(const SandSprite& sprite);