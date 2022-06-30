#pragma once

#include "Leveling.h"
#include "Rendering.h"
#include "Sand/Sand.h"

struct Sand_System_UpdateLineProjectileMesh : SystemBase
{
private:
	Entity meshEntity;
	SandWorld* sand;

public:
	void Init()
	{
		sand = &GetModule<SandWorld>();

		r<Buffer> float6 = mkr<Buffer>(Buffer(0, 6, Buffer::_f32, DYNAMIC_HOST));

		Mesh mesh = Mesh(DYNAMIC_HOST)
			.SetTopology(Mesh::tLines)
			.Add(Mesh::aPosition, 0, 2, float6)
			.Add(Mesh::aColor, 0, 4, float6)
			.SetOffset(Mesh::aColor, sizeof(vec2));

		meshEntity = CreateEntity().AddAll(Transform2D(), Mesh(mesh));
	}

	void Dnit()
	{
		meshEntity.Destroy();
	}

	void Update()
	{
		for (auto [entity, transform, proj] : QueryWithEntity<Transform2D, CellProjectile>())
		{
			Transform2D last = transform.LastTransform();
			vec2 delta = transform.position - last.position;

			float ticks = length(delta) * 10.f; // todo: this is sand world cells per meter / 2
			vec2 deltaTick = delta / ticks;

			for (float t = 0; t < ticks; t += 1.f)
			{
				last.position += deltaTick;
								
				PokeCircleResult results = PokeCircle(last.position, proj.owner, 0);
				for (const auto& result : results.hits)
				if (result.hasHit)
				{
					Send(event_Sand_ProjectileHit{ 
						entity, 
						result.hit.hitEntity, 
						result.hit.hitIndex, 
						result.hit.hitPos 
					});
				}
			}
		}
	}

private:

	struct PokeHitInfo
	{
		Entity hitEntity;
		ivec2 hitIndex;
		vec2 hitPos;
	};

	struct PokePointResult
	{
		bool onScreen;
		bool hasHit;
		int hitStrength;
		PokeHitInfo hit;
	};

	struct PokeCircleResult
	{
		std::vector<PokePointResult> hits;
	};

	struct PokeLineResult
	{
		int finalHealth;
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
						result.hasHit = true;
						result.hit = PokeHitInfo{ hitEntity, hitInfo.spriteHitIndex, p };
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

	PokeLineResult PokeLine(vec2 a, vec2 b, u32 owner, float turnOnHitRate, int health)
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

	//std::pair<vec2, vec2> DrawLine(SandWorld& sand, Entity e, Cell& cell)
	//{
	//	vec2 delta = cell.vel * Time::DeltaTime();
	//	vec2 origin = cell.pos;

	//	float distance = glm::length(delta);
	//	delta /= distance;
	//	delta /= sand.cellsPerMeter;

	//	bool isProjectile = e.Has<CellProjectile>();

	//	for (int i = 0; i < ceil(distance); i++, cell.pos += delta)
	//	{
	//		if (isProjectile && sand.OnScreen(cell.pos))
	//		{
	//			CellProjectile& proj = e.Get<CellProjectile>();
	//			
	//			// trail
	//			CreateEntity().Add<Cell>(cell.pos, vec2(0.f, 0.f), cell.color, proj.trailLife);

	//			const ivec4& spriteInfo = sand.GetCollisionInfo(cell.pos);
	//			if (spriteInfo[3] > 0)
	//			{
	//				Entity tileEntity = Wrap(spriteInfo[2]);
	//				
	//				if (  !tileEntity.IsAlive()
	//					|| tileEntity.Id() == proj.owner)
	//				{
	//					continue;
	//				}

	//				SandSprite& sprite = tileEntity.Get<SandSprite>();
	//				
	//				if (sprite.invulnerable)
	//				{
	//					cell.life = 0;
	//					break;
	//				}
	//				
	//				Send(event_Sand_ProjectileHit{ tileEntity, e, ivec2(spriteInfo[0], spriteInfo[1]), cell.pos });

	//				int& health = e.Get<CellProjectile>().health;
	//				health -= sprite.cellStrength;
	//				
	//				if (health <= 0)
	//				{
	//					cell.life = 0;
	//					break;
	//				}

	//				// turn projectile a little
	//				vec2& v = cell.vel;
	//				float d = length(v);
	//				v = normalize(v + get_randn(d * proj.turnOnHitRate)) * d;
	//				delta = normalize(cell.vel * Time::DeltaTime()) / sand.cellsPerMeter;
	//			}
	//		}
	//	}

	//	return { origin, cell.pos };
	//}
};