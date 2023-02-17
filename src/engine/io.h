#pragma once

#include <string>

//
//  If a path is a fullpath or a relitive path
//	Returns a fullpath into the ASSET_ROOT_FOLDER
//
std::pair<bool, std::string> IsPathRelative(const std::string& path);

// takes an image file and creates a .atlas file that points to it with default tiling
//
// it will name the file like this
// example.png -> example.png.atlas
//
//	returns the new filename
//
std::string CreateTextureAtlasFileFromImageFile(const std::string& textureFilename);

// take a unqualified name of a dll, and pre/append the underlying OS's
// preferred naming scheme

std::string GetQualifiedLibraryName(const std::string& unqualified);

std::string GetPremake5Command();
std::string GetBuildCommand(const std::string& solutionFilename);
