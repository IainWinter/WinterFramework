#pragma once

#include "Leveling.h"
#include "ext/rendering/Particle.h"

struct System_ParticleUpdate : SystemBase
{
	void Update()
	{
		// engine effects

		for (auto [transform, particle, shrink] : Query<Transform2D, Particle, ParticleShrinkWithAge>())
		{
			transform.scale = particle.orignal.scale * shrink.scale * particle.AgeLeft();
		}

		// particle frame update

		for (auto [entity, particle] : QueryWithEntity<Particle>())
		{
			particle.TickFrame(Time::DeltaTime());
			if (particle.EndOfLife())
			{
				entity.Destroy();
			}
		}

		// emitter update

		for (auto [transform, emitter] : Query<Transform2D, ParticleEmitter>())
		{
			emitter.currentTime += Time::DeltaTime();
			while (emitter.currentTime >= 0)
			{
				emitter.currentTime -= emitter.timeBetweenSpawn;
				SpawnParticle(transform, emitter);
			}
		}
	}

private:

	void SpawnParticle(Transform2D& transform, ParticleEmitter& emitter)
	{
		Particle particle = emitter.Emit(transform);
		
		Entity entity = CreateEntity();
		entity.Add<Particle>(particle);
		entity.Add<Transform2D>(particle.orignal);

		entity.Add<ParticleShrinkWithAge>();

		Rigidbody2D&  body = GetModule<PhysicsWorld>().AddEntity(entity);
		body.SetAngularVelocity(get_randc(10.f));
		body.SetAngle(get_rand(w2PI));
		body.SetVelocity(get_randn(20.f));
		body.SetDamping(5.f);
	}
};