#pragma once

#include "Leveling.h"
#include "Rendering.h"

struct SimpleTriangleRenderer2D : SystemBase
{
	void Update() override
	{
		auto [camera, render] = GetModules<Camera, TriangleRenderer2D>();

		render.Begin(camera, false);
		for (auto [transform, mesh] : Query<Transform2D, Mesh>())
		{
			render.DrawMesh(transform, mesh);
		}
	}
};