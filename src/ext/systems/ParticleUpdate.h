#pragma once

#include "app/System.h"
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

		for (auto [transform, particle, scaleWithAge] : Query<Transform2D, Particle, ParticleScaleWithAge>())
		{
			transform.scale = lerp(particle.original.scale, scaleWithAge.scale, particle.AgeLeft());
		}

		// emitter update

		for (auto [transform, emitter] : Query<Transform2D, ParticleEmitter>())
		{
			if (!emitter.enableAutoEmit) continue;
			
			emitter.currentTime += Time::DeltaTime();
			if (emitter.currentTime > emitter.timeBetweenSpawn)
			{
				emitter.currentTime = 0;

				std::vector<Particle> particles = emitter.EmitLine(
					transform.LastTransform().position, 
					transform.position,
					.1f
				);

				for (Particle& p : particles)
				{
					Entity e = CreateEntity();
					e.Add<Particle>(p);
					e.Add<Transform2D>(p.original)
						.SetZIndex(transform.z);

					const auto& func = emitter.spawners.at(p.m_emitterSpawnerIndex).onCreate;
					if (func) func(e);
				}
			}
		}
	}
};