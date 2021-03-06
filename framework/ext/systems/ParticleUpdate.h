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

		for (auto [transform, particle, shrink] : Query<Transform2D, Particle, ParticleShrinkWithAge>())
		{
			transform.scale = particle.original.scale * particle.AgeLeft();
		}

		// emitter update

		for (auto [transform, emitter] : Query<Transform2D, ParticleEmitter>())
		{
			if (!emitter.enableAutoEmit) continue;
			
			std::vector<Particle> particles = emitter.EmitLine(
				transform.LastTransform().position, 
				transform.position
			);

			for (Particle& p : particles)
			{
				Entity e = CreateEntity();
				e.Add<Particle>(p);
				e.Add<Transform2D>(p.original);

				emitter.spawners.at(p.m_emitterSpawnerIndex).onCreate(e);
			}
		}
	}
};