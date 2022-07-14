#pragma once

#include "Leveling.h"
#include "Components/EnemyAI.h"
#include "Components/Throwable.h"
#include "Components/Lightning.h"
#include "Prefabs.h"

struct System_EnemyController : System<System_EnemyController>
{
	void Update()
	{
		//for (auto [entity, bomb] : QueryWithEntity<ExplodeNearTarget>())
		//{
		//	if (bomb.m_tickFuse)
		//	{
		//		if (entity.Has<TurnTwoardsTarget>()) entity.Remove<TurnTwoardsTarget>();
		//		if (entity.Has<Flocker>())           entity.Remove<Flocker>();
		//	}
		//}

		for (auto [body, flocker] : Query<Rigidbody2D, TurnTwoardsTarget>())
		{
			body.SetVelocity(safe_normalize(body.GetVelocity()) * 10.f); // set constant velocity for flockers
		}

		SandWorld& sand = GetModule<SandWorld>();

		for (auto [transform, particle, cell] : Query<Transform2D, ParticleEmitter, CellProjectile>())
		{
			particle.enableAutoEmit = sand.OnScreen(transform.position);
		}

	// enemy spawner (station)

		for (auto [transform, spawner] : Query<Transform2D, EnemySpawner>())
		{
			spawner.timer -= Time::DeltaTime(); // wait for spawn
			if (spawner.timer > 0.f) continue;
			spawner.timer = spawner.delay;

			if (spawner.spawners.size() == 0) continue; // exit if nothing to spawn

			int randomIndex = get_rand((int)spawner.spawners.size());
			auto [point, enemy] = spawner.spawners.at(randomIndex);

			vec2 spawnPoint = rotate(point, transform.rotation) * transform.scale + transform.position;

			Send(event_Enemy_Spawn{ enemy, spawnPoint, true, point });
		}

	// object throwing (base), could seperate into its own system

		for (auto [entity, transform, thrower] : QueryWithEntity<Transform2D, Thrower>())
		{
			if (!thrower.throwing.IsAlive()) // find a new object to throw
			{
				for (auto [throwableEntity, throwableTransform, throwable] : QueryWithEntity<Transform2D, Throwable>())
				{
					if (distance(transform.position, throwableTransform.position) < thrower.grabRadius)
					{
						thrower.SetThrowing(throwableEntity);
						break; // throwable found, exit
					}
				}
			}

			else // if holding something
			{
				vec2 throwingPos = thrower.throwing.Get<Transform2D>().position;
				float dist = distance(transform.position, throwingPos);

				if (dist > thrower.grabRadius) // holding an object, but it has gone out of range
				{
					thrower.throwing = Entity(); // drop
				}

				else // throw twoards target, would be cool if it tried to orbit the object 
				{    //a little, but this needs more work, for now just apply force

					vec2 delta = safe_normalize(thrower.target.Get<Transform2D>().position - throwingPos);
					Rigidbody2D& body = thrower.throwing.Get<Rigidbody2D>();
					body.ApplyForce(delta * thrower.throwingForce * body.GetMass() / 2.f);

					// spawn lightning connecting the two shapes

					CreateEntity().AddAll(
						Transform2D(transform.position),
						ParticleEmitter(GetPrefab_LightningEmitter()),
						Lightning{ entity, thrower.throwing, thrower.grabRadius + 2.f });
				}
			}
		}
	}
};