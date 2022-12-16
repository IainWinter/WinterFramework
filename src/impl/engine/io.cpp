#include "engine/io.h"

#include "ext/serial/serial_json.h"
#include "ext/rendering/TextureAtlas.h"
#include <filesystem>
#include <fstream>

std::pair<bool, std::string> IsPathRelative(const std::string& path)
{
	std::filesystem::path p(path);
	std::filesystem::path assets(File::GetAssetPath());
	
	// this sets a global state?
	std::filesystem::current_path(assets);

	return { p.is_relative(), std::filesystem::absolute(p).string() };
}


std::string CreateTextureAtlasFileFromImageFile(const std::string& textureFilename)
{
	std::string realFilepath = IsPathRelative(textureFilename).second;

	// create an atlas that links to a texture without loading the texture data
	TextureAtlas dummy;
	dummy.source = Asset::Dummy<Texture>(realFilepath);
	dummy.SetAutoTile(1, 1);

	std::string atlasFilename = realFilepath + ".atlas";

	// write the .atlas file
	json_writer(std::ofstream(atlasFilename)).write(dummy);

	return atlasFilename;
}
