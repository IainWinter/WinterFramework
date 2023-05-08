#pragma once

#include "Rendering.h"

struct ParticleData
{
	vec2 position;
	vec2 scale;
	Color tint;
	float life;
	int textureIndex;
};

// A particle system which stores a tightly packed list of 
// particles using a mesh for rendering
//
class ParticleSystem
{
public:


private:
	std::vector<ParticleData> m_particles;
};