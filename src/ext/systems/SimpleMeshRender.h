#pragma once

#include "Leveling.h"
#include "Rendering.h"

struct SimpleMeshRenderer2D : SystemBase
{
private:
	r<MeshRenderer2D> m_render;

public:
	void Init() override
	{
		m_render = mkr<MeshRenderer2D>();
	}

	void Update() override
	{
		auto [camera] = GetModules<Camera>();

		m_render->Begin(camera, false);
		for (auto [transform, mesh] : Query<Transform2D, Mesh>())
		{
			// need to check for if mesh is 3d or not, maybe have a dimension in mesh or a simple typedef to distinguish
			m_render->DrawMesh(transform, mesh);
		}
	}
};