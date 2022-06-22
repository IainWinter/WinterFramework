#pragma once

#include "Leveling.h"
#include "Events.h"
#include "Sand/Sand.h"
#include "ext/rendering/Particle.h"

struct System_ExplosionSpawner : System<System_ExplosionSpawner>
{
	void Init()
	{
		Attach<event_SpawnExplosion>();

		m_explosion = Particle(
			mkr<TextureAtlas>(mkr<Texture>(_a("explosion.png")), 5, 5), 22
		);
	}

	void on(event_SpawnExplosion& e)
	{
		SpawnExplosion(e.position, e.power);
	}

private:

	Particle m_explosion;

	void SpawnExplosion(vec2 position, float power)
	{
		const auto onCreate = [](Entity& e)
		{
			e.Add<CellProjectile>(0u, 35, .8f);
		};

		float radius = 700.f;

		for (float i = 0; i < power; i += 1.f) // global power scale
		{
			float speed = get_rand(radius) + radius / 3.f;
			vec2 velocity = get_randn(speed);
			Send(event_Sand_CreateCell(position, velocity, Color(205 + get_rand(50), 218, 20), .2f, onCreate));
		}

		CreateEntity().AddAll(
			Transform2D (position, vec2(6.f, 6.f), get_rand(2.f * pi<float>())),
			Particle    (m_explosion)
		);
	}
};