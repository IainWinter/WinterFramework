#pragma once

#include "Rendering.h"
#include "ext/AssetStore.h"
#include "ext/rendering/Sprite.h"

struct TextureAtlas
{
	struct Bounds
	{
		vec2 uvOffset = vec2(0.f);
		vec2 uvScale  = vec2(1.f);
	};

	a<Texture> source;
	std::vector<Bounds> bounds;

	TextureAtlas() = default;
	TextureAtlas(a<Texture> source, const std::vector<Bounds>& bounds);
	TextureAtlas(a<Texture> source);
	TextureAtlas(const std::string& filename);

	TextureAtlas& SetAutoTile(int numberOfTilesX, int numberOfTilesY);

	const Bounds& GetUVForFrame(int frame) const;
	Sprite GetSpriteForFrame(int frame) const;

	int GetFrameCount() const;
};
