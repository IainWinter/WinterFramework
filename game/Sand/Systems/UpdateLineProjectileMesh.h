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
		// delete old cells
		for (auto [e, cell] : QueryWithEntity<Cell>())
		{
			if (cell.life >= 0)
			{
				cell.life -= Time::DeltaTime();
				if (cell.life <= 0) e.DestroyAtEndOfFrame();
			}
		}
	}

	void FixedUpdate()
	{
		struct LineToDraw
		{
			vec2 a; vec4 colorA;
			vec2 b; vec4 colorB;
		};

		std::vector<LineToDraw> toDraw;

		for (auto [e, tran, cell] : QueryWithEntity<Transform2D, Cell>())
		{
			LineToDraw draw;
			draw.a = tran.position;
			draw.b = tran.position + cell.vel * Time::FixedTime();
			draw.colorA = cell.color.as_v4();
			draw.colorB = cell.color.as_v4();

			if (e.Has<CellProjectile>())
			{
				CellProjectile& proj = e.Get<CellProjectile>();
				PokeLineResult result = PokeLine(draw.a, draw.b, proj.owner, proj.turnOnHitRate, proj.health);

				for (const PokeHitInfo& info : result.hits)
				{
					Send(event_Sand_ProjectileHit{ e, info.hitEntity, info.hitIndex, info.hitPos });
				}

				cell.vel = result.finalVelocity / Time::FixedTime();
				proj.health = result.finalHealth;
				
				if (proj.health <= 0)
				{
					cell.life = 0;
				}

				draw.b = result.finalPosition;
			}

			tran.position = draw.b;

			float dist = distance(draw.a, draw.b);
			if (dist > 10)
			{
				continue;
			}

			toDraw.push_back(draw);
		}

		meshEntity.Get<Mesh>().Get(Mesh::aPosition)->Set(toDraw.size() * 2, toDraw.data());
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