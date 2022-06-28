#pragma once

#include "Leveling.h"
#include "Rendering.h"
#include "Sand/Sand.h"

struct Sand_System_UpdateLineProjectileMesh : SystemBase
{
private:
	Entity meshEntity;

public:
	void Init()
	{
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
			if (cell.life > 0)
			{
				cell.life -= Time::DeltaTime();
				if (cell.life <= 0) e.Destroy();
			}
		}
	}

	void FixedUpdate()
	{
		SandWorld& sand = GetModule<SandWorld>();
		Texture& collisionMaskRender = *sand.screenRead->Get(Target::aColor); // sprite info / collision info, probally an index (or entity index) to an array of structs

		struct LineToDraw
		{
			vec2 a; vec4 colorA;
			vec2 b; vec4 colorB;
		};

		std::vector<LineToDraw> toDraw;

		for (auto [e, tran, body, cell] : QueryWithEntity<Transform2D, Rigidbody2D, Cell>())
		{
			LineToDraw draw;
			draw.a = body.GetPosition();
			draw.b = body.GetPosition() + body.GetVelocity() * Time::FixedTime();
			draw.colorA = cell.color.as_v4();
			draw.colorB = cell.color.as_v4();

			if (e.Has<CellProjectile>())
			{
				CellProjectile& proj = e.Get<CellProjectile>();
				PokeLineResult result = PokeLine(draw.a, draw.b, proj.owner, proj.turnOnHitRate);

				draw.b = result.finalPosition;
				//body.SetVelocity(result.finalVelocity);

				for (const PokeHitInfo& info : result.hits)
				{
					Send(event_Sand_ProjectileHit{ e, info.hitEntity, info.hitIndex, info.hitPos });

					proj.health -= 1;
					if (proj.health <= 0)
					{
						e.Destroy();
						break;
					}
				}
			}

			toDraw.push_back(draw);
		}

		meshEntity.Get<Mesh>().Get(Mesh::aPosition)->Set(toDraw.size() * 2, toDraw.data());

		//std::vector<vec2> verticesOfLines;
		//std::vector<vec4>   colorsOfLines;

		//for (auto [e, cell] : QueryWithEntity<Cell>())
		//{
		//	auto [a, b] = DrawLine(sand, e, cell);
		//	float dist = distance(a, b);

		//	vec2 smallest = vec2(1.f / sand.cellsPerMeter);
		//	if (dist < length(smallest))
		//	{
		//		b += smallest;
		//	}

		//	verticesOfLines.push_back(a);
		//	verticesOfLines.push_back(b);

		//	colorsOfLines.push_back(cell.color.as_v4());
		//	colorsOfLines.push_back(cell.color.as_v4());
		//}

		//Mesh& lines = entity.Get<Mesh>();
		//lines.Get(Mesh::aPosition)->Set(verticesOfLines);
		//lines.Get(Mesh::aColor)   ->Set(colorsOfLines);

		//Transform2D& transform = entity.Get<Transform2D>();
		//transform.scale = sand.worldScale;
	}

private:

	struct PokeHitInfo
	{
		Entity hitEntity;
		ivec2 hitIndex;
		vec2 hitPos;
	};

	struct PokeLineResult
	{
		vec2 finalPosition;
		vec2 finalVelocity;
		std::vector<PokeHitInfo> hits;
	};

	PokeHitInfo PokePoint(vec2 p, u32 owner)
	{

	}

	PokeLineResult PokeLine(vec2 a, vec2 b, u32 owner, float turnOnHitRate)
	{
		PokeLineResult out;

		SandWorld& sand = GetModule<SandWorld>();

		vec2 vel = b - a;
		vec2 delta = vel;
		float distance = length(delta);
		
		if (distance > .001f)
		{
			delta /= distance * sand.cellsPerMeter;

			int i = 0, count = ceil(distance);
			while (i <= count)
			{
				if (!sand.OnScreen(a))
				{
					break; // exit off screen
				}

				CellCollisionInfo hitInfo = sand.GetCollisionInfo(a);

				if (hitInfo.hasHit)
				{
					Entity hitEntity = Wrap(hitInfo.spriteEntityID);

					if (hitEntity.IsAlive() && hitEntity.Id() != owner)
					{
						SandSprite& sprite = hitEntity.Get<SandSprite>();

						if (sprite.invulnerable)
						{
							break; // exit at current position if hit invulnerable
						}

						out.hits.push_back({ hitEntity, hitInfo.spriteHitIndex, a }); // add to hits

						// turn projectile a little
						float len = length(vel);
						vel = normalize(vel + get_randn(len * turnOnHitRate)) * len;
						delta /= len * sand.cellsPerMeter;  // code reuse...
					}
				}

				i += 1;
				a += delta;
			}
		}

		out.finalPosition = a;
		out.finalVelocity = vel;
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