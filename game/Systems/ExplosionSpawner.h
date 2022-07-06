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

		m_smoke = Particle(
			mkr<TextureAtlas>(mkr<Texture>(_a("smoke.png")))
		);

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
		const auto onCreate = [](Entity& e)
		{
			e.Add<CellProjectile>(0u, 35, .8f);
		};

		float radius = 10.f;

		for (float i = 0; i < power; i += .1f) // global power scale
		{
			Entity e = CreateEntity();
			e.Add<Transform2D>(position);
			e.Add<CellProjectile>(u32(-1));
			e.Add<DestroyInTime>(1.f);

			GetModule<PhysicsWorld>()
				.AddEntity(e)
				.SetVelocity(get_randn(radius))
				.SetDamping(1.f);

			e.Add<ParticleEmitter>(GetPrefab_BulletEmitter());// .spawners.at(0).particle.original.scale = vec2(.4, .5);
		
			//Entity smoke = CreateEntity().AddAll(
			//	Transform2D(position, vec2(.5f, .5f), get_rand(w2PI)),
			//	Particle(m_smoke)
			//);

			//GetModule<PhysicsWorld>()
			//	.AddEntity(smoke)
			//	.SetVelocity(get_randn(radius / 2.f))
			//	.SetDamping(4.f);
		}
	}
};