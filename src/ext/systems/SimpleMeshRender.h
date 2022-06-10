#pragma once

#include "Leveling.h"
#include "Rendering.h"

struct SimpleMeshRenderer2D : SystemBase
{
	void Update() override
	{
		auto [camera, render] = GetModules<Camera, MeshRenderer2D>();

		render.Begin(camera, false);
		for (auto [transform, mesh] : Query<Transform2D, Mesh>())
		{
			// need to check for if mesh is 3d or not, maybe have a dimension in mesh or a simple typedef to distinguish
			render.DrawMesh(transform, mesh);
		}
	}
};