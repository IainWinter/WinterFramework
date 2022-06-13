#pragma once

#include "Leveling.h"
#include "Events.h"
#include "Sand.h"

struct System_ExplosionSpawner : System<System_ExplosionSpawner>
{
	void Init()
	{
		Attach<event_SpawnExplosion>();
	}

	void on(event_SpawnExplosion& e)
	{
		SpawnExplosion(e.position, e.power);
	}

private:

	void SpawnExplosion(vec2 position, float power)
	{
		const auto onCreate = [](Entity& e)
		{
			e.Add<CellProjectile>(0u);
			e.Add<CellLife>(get_rand(.4f) + .1f);
		};

		float r = 500.f;

		for (float i = 0; i < power; i += 1.f) // global power scale
		{
			float speed = get_rand(r) + r / 3.f;
			vec2 velocity = get_randn(1.f) * speed;
			Send(event_SpawnSandCell { position, velocity, Color(150, 50, 50), onCreate });
		}
	}
};