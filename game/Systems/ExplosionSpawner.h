#pragma once

#include "app/System.h"
#include "Sand/Sand.h"
#include "ext/rendering/Particle.h"
#include "Events.h"
#include "Prefabs.h"

struct System_ExplosionSpawner : System<System_ExplosionSpawner>
{
	void Init()
	{
		Attach<event_SpawnExplosion>();

		m_smoke = Particle(
			mkr<TextureAtlas>(mkr<Texture>(_a("smoke.png")))
		);

		m_smoke.AddTint(Color(255, 255, 255, 255));
		m_smoke.AddTint(Color(255, 255, 255, 0));

		m_smoke.repeatCount = 50;
	}

	void on(event_SpawnExplosion& e)
	{
		SpawnExplosion(e.position, e.power);
	}

private:

	Particle m_smoke;

	void SpawnExplosion(vec2 position, float power)
	{
		float radius = 10.f;

		for (float i = 0; i < power; i += .1f) // global power scale
		{
			Entity e = CreateEntity();
			e.Add<Transform2D>(position);
			e.Add<CellProjectile>();
			e.Add<DestroyInTime>(.1f + get_rand(.3f));

			e.Add<Rigidbody2D>()
				.SetVelocity(get_randn(radius))
				.SetDamping(radius / 10.f);

			e.Add<ParticleEmitter>(GetPrefab_BulletEmitter());

		}

		for (int i = 0; i < 5; i++)
		{
			Entity smoke = CreateEntity().AddAll(
				Transform2D(position + get_randn(.1f), vec2(2.5f, 2.5f), get_rand(w2PI)),
				Particle(m_smoke)
			);

			smoke.Add<Rigidbody2D>()
				.SetVelocity(get_randn(radius))
				.SetDamping(3.f);
		}
	}
};