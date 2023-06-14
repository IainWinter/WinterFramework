#include "io/AtlasFromImage.h"
#include "ext/AssetStore.h"
#include "ext/rendering/TextureAtlas.h"
#include "ext/serial/serial_json.h"
#include <fstream>

void io_CreateAtlasFromImageOnDisk(const std::string& image, const std::string& atlasOut, int numberOfTilesX, int numberOfTilesY)
{
	// Could fake this
	a<Texture> source = Asset::LoadFromFile<Texture>(image);

	TextureAtlas atlas = TextureAtlas(source)
		.SetAutoTile(numberOfTilesX, numberOfTilesY);

	std::ofstream out(atlasOut);

	if (out.is_open())
		json_writer(out).write(atlas);
}
