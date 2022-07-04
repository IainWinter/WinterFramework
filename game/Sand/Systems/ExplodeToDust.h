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
		auto [transform, sprite] = explodeMe.GetAll<Transform2D, Sprite>();
		Texture& tex = sprite.Get();
		vec2 mid = vec2(tex.Width(), tex.Height()) / 2.f;

		SandWorld& sand = GetModule<SandWorld>(); // non threadsafe read

		// delta for each pixel in world space

		vec2 scale = vec2(1.f) / sand.cellsPerMeter;
		vec2 dx = rotate(vec2(1.f, 0.f), transform.rotation) * scale * 2.f;
		vec2 dy = rotate(vec2(0.f, 1.f), transform.rotation) * scale * 2.f;

		printf("%f %f\n", dx.x, dy.y);

		for (const int& index : island)
		{
			auto [x, y] = get_xy(index, tex.Width());
			vec2 pos = (vec2(x, y) - mid) * vec2(dx.x, dy.y) + transform.position;

			Entity dust = CreateEntity();
			dust.Add<Transform2D>(vec3(pos, transform.z), scale);
			dust.Add<Particle>()
				.AddTint(Color(255, 0, 0))
				.SetRepeatCount(10000);
		}
		
		////if (false) // disable, should create particles
		//for (const int& index : island)
		//{
		//	auto [x, y] = get_xy(index, tex.Width());

		//	vec2 pos = (vec2(x, y) - mid) / 10.f;
		//	rotate(pos, transform.rotation);
		//	pos += transform.position;

		//	vec2 scale = vec2(1.f / sand.cellsPerMeter); // non threadsafe read
		//	Color& color = tex.At(x, y);

		//	Entity dust = CreateEntity();
		//	dust.Add<Transform2D>(vec3(pos, transform.z), scale);
		//	dust.Add<Particle>()
		//		.AddTint(Color(255, 0, 0))
		//		.SetRepeatCount(10000);
		//}

		if (destroyAll)
		{
			explodeMe.DestroyAtEndOfFrame();
		}
	}
};