#pragma once

#include "Leveling.h"
#include "Sand/SandEvents.h"
#include "Sand/Sand.h"
#include "Sand/SandHelpers.h"

struct Sand_System_ExplodeToDust : System<Sand_System_ExplodeToDust>
{
	void Init()
	{
		Attach<event_Sand_ExplodeToDust>();
	}

	void on(event_Sand_ExplodeToDust& e)
	{
		bool destroyAll = e.onlyThisIndex.size() == 0;
		if (destroyAll) e.onlyThisIndex = GetCorePixels(e.entity.Get<Sprite>().source).all;
		ExplodeSpriteIntoDust(e.onlyThisIndex, e.entity, destroyAll);
	}

private:

	void ExplodeSpriteIntoDust(const std::vector<int>& island, Entity explodeMe, bool destroyAll)
	{
		//auto [transform, sprite] = explodeMe.GetAll<Transform2D, Sprite>();
		//Texture& tex = sprite.Get();
		//vec2 mid = vec2(tex.Width(), tex.Height()) / 2.f;

		//SandWorld& sand = GetModule<SandWorld>(); // non threadsafe read

		//if (false) // disable, should create particles
		//for (const int& index : island)
		//{
		//	auto [x, y] = get_xy(index, tex.Width());

		//	vec2 pos = (vec2(x, y) - mid) / 10.f;
		//	rotate(pos, transform.rotation);
		//	pos += transform.position;

		//	Color& color = tex.At(x, y);
		//	vec2 offset = vec2(1.f / sand.cellsPerMeter); // non threadsafe read

		//	vec2 vels[4] = {
		//		get_randc(3, 3),
		//		get_randc(3, 3),
		//		get_randc(3, 3),
		//		get_randc(3, 3)
		//	};

		//	//Send(event_Sand_CreateCell(pos, vels[0], color, get_rand(.3f) + .3f));
		//	//Send(event_Sand_CreateCell(pos, vels[1], color, get_rand(.3f) + .3f));
		//	//Send(event_Sand_CreateCell(pos, vels[2], color, get_rand(.3f) + .3f));
		//	//Send(event_Sand_CreateCell(pos, vels[3], color, get_rand(.3f) + .3f));
		//}

		if (destroyAll)
		{
			explodeMe.DestroyAtEndOfFrame();
		}
	}
};