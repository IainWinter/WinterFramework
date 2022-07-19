#pragma once

#include "Physics.h"
#include "Sand/Sand.h"
#include "app/System.h"
#include "ext/rendering/Particle.h"
#include "Components/Lightning.h"
#include "ext/physics/ClosestPointOnShape.h"

struct System_Lightning : SystemBase
{
	void Update()
	{
		auto [physics, sand] = GetModules<PhysicsWorld, SandWorld>();

		for (auto [entity, transform, emitter, lightning] : QueryWithEntity<Transform2D, ParticleEmitter, Lightning>())
		{
			vec2 targetPosition = lightning.target.Get<Transform2D>().position;
			vec2 delta = targetPosition - transform.position;
			float targetDistance = clamp(length(delta), 0, lightning.maxDistance);
			targetPosition = transform.position + safe_normalize(delta) * targetDistance;

			PointQueryResult query = physics.QueryPoint(targetPosition, 3.f);
			
			// remove owner of lightning
			for (auto itr = query.results.begin(); itr != query.results.end(); ++itr)
			{
				if (itr->entity == lightning.owner)
				{
					query.results.erase(itr);
					break;
				}
			}

			if (query.results.size() > 0)
			{
				int randomIndex = get_rand((int)query.results.size() - 1);
				PointQueryResult::Result result = query.results.at(randomIndex);

				if (result.entity.Has<Rigidbody2D>())
				{
					targetPosition = GetClosestPoint(result.entity.Get<Rigidbody2D>(), transform.position);

					if (result.entity.Has<SandSprite>())
					{
						SandSprite& sprite = query.FirstEntiy().Get<SandSprite>();

						for (int x = -5; x < 5; x++)
						for (int y = -5; y < 5; y++)
						{
							vec2 p = targetPosition + vec2(x, y) / sand.cellsPerMeter;

							if (sand.OnScreen(p))
							{
								CellCollisionInfo info = sand.GetCollisionInfo(p);

								if (info.hasHit)
								{
									if (entity.Has<LightningDamage>())
									{
										int index1d = sprite.colliderMask->Index32(info.spriteHitIndex.x, info.spriteHitIndex.y);
										Send(event_Sand_RemoveCell{ Wrap(info.spriteEntityID), index1d, p });
									}

									targetPosition = p;
									goto exit;
								}
							}
						}
					}
				}
			}
			
exit:

			EmitLightningInOneFrame(
				emitter, 
				transform.position, 
				targetPosition,
				lightning.arcDeviation
			);

			entity.Destroy();
		}
	}

private:

	vec2 EmitLightningSegment(ParticleEmitter& emitter, vec2 position, vec2 target, float arc)
	{
		vec2 normal = target - position;
		float radius = length(normal) * (get_rand(.5f) + .5);
		float ang = atan2(normal.y, normal.x) + (get_rand(arc * 2) - arc);
		vec2 nextPosition = position + vec2(cos(ang), sin(ang)) * radius * .5f;

		emitter.EmitLine(position, nextPosition);

		return nextPosition;
	}

	vec2 EmitLightningInOneFrame(ParticleEmitter& emitter, vec2 position, vec2 target, float arc)
	{
		// make a circle with origin position and radius length(target - position)
		// calc the angle between pos and target
		// add a random amount to this angle and set next pos to the point on the circle at this adjusted angle
		// repeat

		for (int i = 0; i < 8; i++)
		{
			position = EmitLightningSegment(emitter, position, target, arc);
		}

		return position;
	}
};