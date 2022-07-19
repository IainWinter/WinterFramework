#pragma once

#include "app/System.h"
#include "Rendering.h"
#include "Sand/Sand.h"

struct Sand_System_UpdateLineProjectileMesh : SystemBase
{
private:
	SandWorld* sand;

public:
	void Init()
	{
		sand = &GetModule<SandWorld>();
	}

	void Update()
	{
		for (auto [entity, transform, body, proj] : QueryWithEntity<Transform2D, Rigidbody2D, CellProjectile>())
		{
			Transform2D last = transform.LastTransform();
			vec2 delta = transform.position - last.position;

			float dist = length(delta);
			float ticks = dist * 10.f; // todo: this is sand world cells per meter / 2
			vec2 deltaTick = delta / ticks;

			for (float t = 0; t < ticks; t += 1.f)
			{
				last.position += deltaTick;
				
				PokeCircleResult results = PokeCircle(last.position, proj.owner, proj.size);
				bool hasHit = false;
				
				float totalHitStrength = 0.f;
				int hitCount = 0;

				for (const auto& result : results.hits)
				if (result.hasHit)
				{
					SendNow(event_Sand_ProjectileHit{ 
						entity, 
						result.hit.hitEntity, 
						result.hit.hitIndex,
						result.hit.hitIndex2D,
						result.hit.hitPos
					});

					hasHit |= result.hasHit;

					totalHitStrength += result.hitStrength;
					hitCount += 1;

					const SandSprite& ssprite = result.hit.hitEntity.Get<SandSprite>();
					if (ssprite.isHardCore && ssprite.pixels.IsInCore(result.hit.hitIndex))
					{
						entity.DestroyAtEndOfFrame();
						break;
					}
				}

				if (hasHit)
				{
					delta = dist * safe_normalize(delta + get_randn(dist * proj.turnOnHitRate));
					//deltaTick = delta / ticks;

					// causes issues with the exit pos not equal to the actual pos

					proj.health -= totalHitStrength / hitCount; // take average
					if (proj.health <= 0)
					{
						entity.DestroyAtEndOfFrame();
						break;
					}
				}
			}

			// set exit velocity to the turned delta

			if (ticks > 0)
			{
				//body.SetPosition(last.position);
				body.SetVelocity(length(body.GetVelocity()) * safe_normalize(delta));
			}
		}
	}

private:

	struct PokeHitInfo
	{
		Entity hitEntity;
		ivec2 hitIndex2D;
		int hitIndex;
		vec2 hitPos;
	};

	struct PokePointResult
	{
		bool onScreen;
		bool hasHit;
		float hitStrength;
		PokeHitInfo hit;
	};

	struct PokeCircleResult
	{
		std::vector<PokePointResult> hits;
	};

	struct PokeLineResult
	{
		float finalHealth;
		vec2 finalPosition;
		vec2 finalVelocity;
		std::vector<PokeHitInfo> hits;
	};

	PokePointResult PokePoint(vec2 p, u32 owner)
	{
		PokePointResult result;
		result.onScreen = sand->OnScreen(p);
		result.hasHit = false;
		result.hitStrength = 0;

		if (result.onScreen)
		{
			CellCollisionInfo& hitInfo = sand->GetCollisionInfo(p);

			if (hitInfo.hasHit)
			{
				hitInfo.hasHit = 0; // remove from this frame

				Entity hitEntity = Wrap(hitInfo.spriteEntityID);

				if (hitEntity.IsAlive() && hitEntity.Id() != owner)
				{
					SandSprite& sprite = hitEntity.Get<SandSprite>();

					if (!sprite.invulnerable)
					{
						int index1d = sprite.Get().Index32(hitInfo.spriteHitIndex.x, hitInfo.spriteHitIndex.y);

						result.hasHit = true;
						result.hit = PokeHitInfo{ hitEntity, hitInfo.spriteHitIndex, index1d, p };
						result.hitStrength = sprite.cellStrength;
					}
				}
			}
		}

		return result;
	}

	PokeCircleResult PokeCircle(vec2 p, u32 owner, int radius)
	{
		PokeCircleResult result;

		for (int y = -radius; y <= radius; y++)
		{
			int layerWidth = radius - abs(y);
			for (int x = -layerWidth; x <= layerWidth; x++)
			{
				result.hits.push_back(PokePoint(p + vec2(x, y) / sand->cellsPerMeter, owner));
			}
		}

		return result;
	}

	PokeLineResult PokeLine(vec2 a, vec2 b, u32 owner, float turnOnHitRate, float health)
	{
		PokeLineResult out;

		vec2 vel = b - a;
		float distance = length(vel);
		float cellDistance = distance * sand->cellsPerMeter;

		vec2 delta = vel;

		if (distance > .001f)
		{
			delta /= cellDistance;

			for (int i = 0; i <= ceil(cellDistance); i += 1, a += delta)
			{
				PokePointResult point = PokePoint(a, owner);

				if (!point.onScreen) break;     // break if off screen
				if (!point.hasHit)   continue;  // continue through air

				out.hits.push_back(point.hit);  // add to hits

				health -= point.hitStrength;
				if (health <= 0) break;         // break if dead

				// turn projectile a little

				delta = normalize(vel + get_randn(distance * turnOnHitRate)) / sand->cellsPerMeter;
			}
		}

		out.finalHealth = health;
		out.finalPosition = a;
		out.finalVelocity = delta * cellDistance;
		return out;
	}
};