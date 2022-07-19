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

	void Update()
	{
		for (auto [entity, dustInTime] : QueryWithEntity<SandTurnToDustInTime>())
		{
			dustInTime.time -= Time::DeltaTime();
			if (dustInTime.time <= 0.f)
			{
				Send(event_Sand_ExplodeToDust{entity});
			}
		}
	}

	void on(event_Sand_ExplodeToDust& e)
	{
		bool destroyAll = e.onlyThisIndex.size() == 0;
		if (destroyAll) e.onlyThisIndex = GetCorePixels(e.entity.Get<Sprite>().source).GetAllAsVec();
		ExplodeSpriteIntoDust(e.onlyThisIndex, e.entity, e.velocity, destroyAll);
	}

private:

	void ExplodeSpriteIntoDust(const std::vector<int>& island, Entity explodeMe, vec2 velocity, bool destroyAll)
	{
		auto [transform, sprite] = explodeMe.GetAll<Transform2D, Sprite>();
		Texture& tex = sprite.Get();
		vec2 mid = vec2(tex.Width(), tex.Height()) / 2.f;

		SandWorld& sand = GetModule<SandWorld>(); // non threadsafe read

		// delta for each pixel in world space
		
		float s = 1.f / sand.cellsPerMeter;
		float r = transform.rotation;
		mat2 coord = mat2(cos(r), sin(r), -sin(r), cos(r))
			       * mat2(2*s, 0, 0, 2*s);

		for (const int& index : island)
		{
			auto [x, y] = get_xy(index, tex.Width());
			vec2 pos = coord * (vec2(x, y) - mid + vec2(.5f, .5f));

			Entity dust = CreateEntity();
			dust.Add<Transform2D>(vec3(pos + transform.position, transform.z), vec2(s), transform.rotation);
			dust.Add<Particle>()
				.AddTint(tex.At(x, y))
				.SetFramesPerSecond(0)
				.SetOriginal(dust.Get<Transform2D>());

			//dust.Add<ParticleShrinkWithAge>();

			dust.Add<Item>()
				.SetType(ITEM_REGOLITH)
				.SetPickupRadius(8.f)
				.SetLife(4.f + get_rand(4.f))
				.SetPickupDelay(.1f + get_rand(.5f));

			vec2 vel = velocity;
			if (explodeMe.Has<Rigidbody2D>())
			{
				vel += explodeMe.Get<Rigidbody2D>().GetVelocity();
			}
			vel += get_randn(length(vel) / 2.f + 1.f);

			GetModule<PhysicsWorld>().AddEntity(dust)
				.SetVelocity(vel)
				.SetAngularVelocity(get_randc(w2PI));
		}

		if (destroyAll)
		{
			explodeMe.DestroyAtEndOfFrame();
		}
	}
};