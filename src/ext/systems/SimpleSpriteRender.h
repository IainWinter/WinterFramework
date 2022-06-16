#pragma once

#include "Leveling.h"
#include "Rendering.h"

struct SimpleSpriteRenderer2D : SystemBase
{
private:
	r<SpriteRenderer2D> m_render;

public:
	void Init() override
	{
		m_render = mkr<SpriteRenderer2D>();
	}

	void Update() override
	{
		auto [camera] = GetModules<Camera>();

		m_render->Begin(camera);
		m_render->Clear();
		for (auto [transform, sprite] : Query<Transform2D, Sprite>())
		{
			m_render->DrawSprite(transform, sprite);
		}
	}
};