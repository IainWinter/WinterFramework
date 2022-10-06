#pragma once

#include "Rendering.h"

struct Model
{
	// should add transform that is for the local meshes
	std::vector<r<Mesh>> meshes;
};
