#pragma once

#include "Leveling.h"
#include "Events.h"
#include "Sand/Sand.h"

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
			e.Add<CellProjectile>(0u, 35, .8f);
		};

		float r = 700.f;

		for (float i = 0; i < power; i += 1.f) // global power scale
		{
			float speed = get_rand(r) + r / 3.f;
			vec2 velocity = get_randn(speed);
			Send(event_Sand_CreateCell(position, velocity, Color(205 + get_rand(50), 218, 20), .4f, onCreate));
		}
	}
};