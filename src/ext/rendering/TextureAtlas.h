#pragma once

#include "Rendering.h"

struct TextureAtlas
{
	struct Bounds
	{
		vec2 uvOffset;
		vec2 uvScale;
	};

	r<Texture> source;
	std::vector<Bounds> bounds;

	TextureAtlas(r<Texture> source, const std::vector<Bounds>& bounds)
		: source (source)
		, bounds (bounds)
	{}

	TextureAtlas(r<Texture> source, int numberOfTilesX = 1, int numberOfTilesY = 1)
		: source (source)
	{
		bounds.reserve(numberOfTilesX * numberOfTilesY);

		vec2 scale  = vec2(1.f, 1.f) / vec2(numberOfTilesX, numberOfTilesY);

		for (int y = 0; y < numberOfTilesY; y++)
		{
			for (int x = 0; x < numberOfTilesX; x++)
			{
				bounds.push_back(Bounds { vec2(x, y) * scale, scale});
			}
		}
	}

	// add a constructor for loading a json file that contains the bounds...

	const Bounds& GetUVForFrame(int frame) const
	{
		assert(frame >= 0 && frame < bounds.size() && "Frame out of bounds");
		return bounds.at(frame);
	}

	int GetFrameCount() const
	{
		return (int)bounds.size();
	}
};
