#pragma once

#include "v2/Render/Texture.h"
#include <unordered_map>

// think about who owns the textures
// where does the width / height get stored
// can only 2d textures be put here
// who gets the set theses as active

//	A map of named Textures
//	There is no concept of this being on the host / device because it is just a collection
//	of textures. Though, it allocates on the device.
// 
// Actually, for software renderer support there should be a host / device separation. HostTargetys can assert that their textures will remain on the host, and vice versa
//
class HostTarget
{
public:
	HostTarget& Add(TargetName name, Texture_New texture);
	HostTarget& Remove(TargetName name);

	Texture_New Get(TargetName name);

private:
	std::unordered_map<TargetName, Texture_New> m_textures;
};

