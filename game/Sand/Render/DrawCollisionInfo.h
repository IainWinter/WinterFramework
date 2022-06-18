#pragma once

#include "Rendering.h"
#include "Sand/SandComponents.h"

struct SandCollisionInfoRenderer
{
	r<ShaderProgram> m_shader;
	r<Mesh>          m_quad;

	SandCollisionInfoRenderer();
	void Begin(Camera& camera, r<Target> target);
	void DrawCollisionInfo(const Transform2D& transform, SandSprite& sprite, int spriteIndex);
};