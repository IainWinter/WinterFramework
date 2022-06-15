#pragma once

#include "Leveling.h"
#include "Rendering.h"

struct SimpleSpriteRenderer2D : SystemBase
{
	void Update() override
	{
		auto [camera, render] = GetModules<Camera, SpriteRenderer2D>();

		render.Begin(camera);
		render.Clear();
		for (auto [transform, sprite] : Query<Transform2D, Sprite>())
		{
			render.DrawSprite(transform, sprite);
		}
	}
};