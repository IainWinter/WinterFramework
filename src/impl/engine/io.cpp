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
    std::ofstream file = std::ofstream(atlasFilename);
    
	// write the .atlas file
	json_writer(file).write(dummy);

	return atlasFilename;
}

std::string GetPremake5Command()
{
	std::filesystem::path tools(wTOOLS);
	std::filesystem::path premake;

#ifdef _WIN32
	premake = "premake5.exe";
#endif

#ifdef __APPLE__
	premake = "mac_premake5";
#endif

#ifdef __linux__
	premake = "premake5";
#endif

	return (tools / premake).make_preferred().string() + " " + wPREMAKE_BUILD;
}

std::string GetBuildCommand(const std::string& solutionFilename)
{
	std::string builCommand;

#ifdef _WIN32
	builCommand = "msbuild " + solutionFilename;
#endif

#ifdef __APPLE__
	builCommand = "needs impl" + solutionFilename;
#endif

#ifdef __linux__
	builCommand = "make";
#endif

	return builCommand;
}