#include "ext/rendering/TextureAtlas.h"
#include "ext/serial/serial_json.h"
#include <fstream>

TextureAtlas::TextureAtlas(a<Texture> source, const std::vector<Bounds>& bounds)
	: source (source)
	, bounds (bounds)
{}

TextureAtlas::TextureAtlas(a<Texture> source)
	: source (source)
{
	SetAutoTile(1, 1);
}

TextureAtlas::TextureAtlas(const std::string& filename)
{
	std::ifstream in(filename);

	if (in.is_open())
	{
		json_reader(in).read(*this);
	}

	SetAutoTile(1, 1);
}

TextureAtlas& TextureAtlas::SetAutoTile(int numberOfTilesX, int numberOfTilesY)
{
	bounds.clear();
	bounds.reserve(numberOfTilesX * numberOfTilesY);

	vec2 scale = vec2(1.f, 1.f) / vec2(numberOfTilesX, numberOfTilesY);

	for (int y = 0; y < numberOfTilesY; y++)
	{
		for (int x = 0; x < numberOfTilesX; x++)
		{
			bounds.push_back(Bounds{ vec2(x, y) * scale, scale });
		}
	}

	return *this;
}

const TextureAtlas::Bounds& TextureAtlas::GetUVForFrame(int frame) const
{
	assert(frame >= 0 && frame < bounds.size() && "Frame out of bounds");
	return bounds.at(frame);
}

Sprite TextureAtlas::GetSpriteForFrame(int frame) const
{
	Bounds bounds = GetUVForFrame(frame);

	return Sprite(source)
		.SetUvOffset(bounds.uvOffset)
		.SetUvScale(bounds.uvScale);
}

int TextureAtlas::GetFrameCount() const
{
	return (int)bounds.size();
}