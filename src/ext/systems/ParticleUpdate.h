#pragma once

#include "Leveling.h"
#include "ext/rendering/Particle.h"

struct System_ParticleUpdate : SystemBase
{
	void Update()
	{
		// particle frame update

		for (auto [entity, particle] : QueryWithEntity<Particle>())
		{
			particle.TickFrame(Time::DeltaTime());
			if (particle.EndOfLife())
			{
				entity.Destroy();
			}
		}

		// engine effects

		for (auto [transform, particle, shrink] : Query<Transform2D, Particle, ParticleShrinkWithAge>())
		{
			transform.scale = particle.original.scale * shrink.scale * particle.AgeLeft();
		}

		// emitter update

		for (auto [transform, emitter] : Query<Transform2D, ParticleEmitter>())
		{
			if (!emitter.enabled) continue;

			Transform2D last = transform.LastTransform();
			vec2 delta = transform.position - last.position;

			float ticks = length(delta) * 10.f; // todo: this is sand world cells per meter / 2
			vec2 deltaTick = delta / ticks;

			for (float t = 0; t < ticks; t += 1.f)
			{
				last.position += deltaTick;
				emitter.Emit(last);
			}

			//emitter.currentTime += Time::DeltaTime();
			//while (emitter.currentTime >= 0)
			//{
			//	emitter.currentTime -= emitter.timeBetweenSpawn;
			//	emitter.Emit(transform);
			//}
		}
	}
};