#pragma once

#include "Leveling.h"
#include "ext/rendering/Particle.h"

struct System_ParticleUpdate : SystemBase
{
	void Update()
	{
		for (auto [entity, particle] : QueryWithEntity<Particle>())
		{
			particle.TickFrame();
			if (particle.EndOfLife())
			{
				entity.Destroy();
			}
		}
	}
};