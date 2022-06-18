#pragma once

#include "Leveling.h"
#include "Rendering.h"
#include "Sand/Render/DrawCollisionInfo.h"
#include "Sand/Sand.h"

struct Sand_System_DrawSandSprites : SystemBase
{
	void Update()
	{
		auto [camera, sand] = GetModules<Camera, SandWorld>();

		maskRender.Begin(camera, sand.screenWrite);

		for (auto [e, transform, sandSprite] : QueryWithEntity<Transform2D, SandSprite>())
		{
			maskRender.DrawCollisionInfo(transform, sandSprite, e.Id());
		}

		std::swap(sand.screenRead, sand.screenWrite);
	}

private:
	SandCollisionInfoRenderer maskRender;
};