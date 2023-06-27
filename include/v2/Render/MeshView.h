#pragma once

#include "v2/Render/MeshLayout.h"

struct MeshView
{


	u8* buffer;
	MeshLayout layout;

	MeshTopology topology;
};