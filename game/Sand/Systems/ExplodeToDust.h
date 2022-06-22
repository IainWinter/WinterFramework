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
		ExplodeSpriteIntoDust(e.onlyThisIndex, e.entity, e.projectile, destroyAll);
	}

private:

	void ExplodeSpriteIntoDust(const std::vector<int>& island, Entity explodeMe, Entity projectile, bool destroyAll)
	{
		auto [transform, sprite] = explodeMe.GetAll<Transform2D, Sprite>();
		Texture& tex = sprite.Get();
		vec2 mid = vec2(tex.Width(), tex.Height()) / 2.f;
		vec2 vel = projectile.IsAlive() ? projectile.Get<Cell>().vel : vec2(0.f, 0.f);

		SandWorld& sand = GetModule<SandWorld>(); // non threadsafe read

		for (const int& index : island)
		{
			auto [x, y] = get_xy(index, tex.Width());

			vec2 pos = (vec2(x, y) - mid) / 10.f;
			rotate(pos, transform.rotation);
			pos += transform.position;

			Color& color = tex.At(x, y);
			vec2 offset = vec2(1.f / sand.cellsPerMeter); // non threadsafe read

			auto get_vel = [vel]()
			{
				//float r = 100.f;
				//vec2 v = vec2(get_randc(r), get_randc(r));
				vec2 v = vel;

				return v*.1f; // x .1 makes this faster??
			};

			vec2 vels[4] = {
				get_vel() + get_randc(50, 50),
				get_vel() + get_randc(50, 50),
				get_vel() + get_randc(50, 50),
				get_vel() + get_randc(50, 50)
			};

			Send(event_Sand_CreateCell(pos, vels[0], color, get_rand(.3f) + .1f));
			Send(event_Sand_CreateCell(pos, vels[1], color, get_rand(.3f) + .1f));
			Send(event_Sand_CreateCell(pos, vels[2], color, get_rand(.3f) + .1f));
			Send(event_Sand_CreateCell(pos, vels[3], color, get_rand(.3f) + .1f));
		}

		if (destroyAll)
		{
			explodeMe.DestroyAtEndOfFrame();
		}
	}
};