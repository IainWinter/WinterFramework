#pragma once

#include "Leveling.h"
#include "Rendering.h"
#include "Sand/Sand.h"

struct Sand_System_UpdateLineProjectileMesh : SystemBase
{
	void Init()
	{
		Mesh mesh = Mesh(false);
		mesh.Add<vec2>(Mesh::aPosition, {});
		mesh.Add<vec4>(Mesh::aColor, {});
		mesh.topology = Mesh::tLines;

		entity = CreateEntity().AddAll(Transform2D(), Mesh(mesh));
	}

	void Dnit()
	{
		entity.Destroy();
	}

	void Update()
	{
		SandWorld& sand = GetModule<SandWorld>();
		Texture& collisionMaskRender = *sand.screenRead->Get(Target::aColor); // sprite info / collision info, probally an index (or entity index) to an array of structs

		std::vector<vec2> verticesOfLines;
		std::vector<vec4>   colorsOfLines;

		for (auto [e, cell] : QueryWithEntity<Cell>())
		{
			if (cell.life > 0.f)
			{
				cell.life -= Time::DeltaTime();
				if (cell.life <= 0.f)
				{
					e.DestroyAtEndOfFrame();
					continue;
				}
			}

			auto [a, b] = DrawLine(sand, e, cell);
			float dist = distance(a, b);

			vec2 smallest = vec2(1.f / sand.cellsPerMeter);
			if (dist < length(smallest))
			{
				b += smallest;
			}

			verticesOfLines.push_back(a);
			verticesOfLines.push_back(b);

			colorsOfLines.push_back(cell.color.as_v4());
			colorsOfLines.push_back(cell.color.as_v4());
		}

		Mesh& lines = entity.Get<Mesh>();
		lines.Get(Mesh::aPosition)->Set(verticesOfLines);
		lines.Get(Mesh::aColor)   ->Set(colorsOfLines);

		Transform2D& transform = entity.Get<Transform2D>();
		//transform.scale = sand.worldScale;
	}

private:
	Entity entity;

	std::pair<vec2, vec2> DrawLine(SandWorld& sand, Entity e, Cell& cell)
	{
		vec2 delta = cell.vel * Time::DeltaTime();
		vec2 origin = cell.pos;

		float distance = glm::length(delta);
		delta /= distance;
		delta /= sand.cellsPerMeter;

		bool isProjectile = e.Has<CellProjectile>();

		for (int i = 0; i < ceil(distance); i++, cell.pos += delta)
		{
			if (isProjectile && sand.OnScreen(cell.pos))
			{
				CellProjectile& proj = e.Get<CellProjectile>();
				
				// trail
				CreateEntity().Add<Cell>(cell.pos, vec2(0.f, 0.f), cell.color, proj.trailLife);

				const ivec4& spriteInfo = sand.GetCollisionInfo(cell.pos);
				if (spriteInfo[3] > 0)
				{
					Entity tileEntity = Wrap(spriteInfo[2]);
					
					if (  !tileEntity.IsAlive()
						|| tileEntity.Id() == proj.owner)
					{
						continue;
					}

					SandSprite& sprite = tileEntity.Get<SandSprite>();
					
					if (sprite.invulnerable)
					{
						cell.life = 0;
						break;
					}
					
					Send(event_Sand_ProjectileHit{ tileEntity, e, ivec2(spriteInfo[0], spriteInfo[1]), cell.pos });

					int& health = e.Get<CellProjectile>().health;
					health -= sprite.cellStrength;
					
					if (health <= 0)
					{
						cell.life = 0;
						break;
					}

					// turn projectile a little
					vec2& v = cell.vel;
					float d = length(v);
					v = normalize(v + get_randn(d * proj.turnOnHitRate)) * d;
					delta = normalize(cell.vel * Time::DeltaTime()) / sand.cellsPerMeter;
				}
			}
		}

		return { origin, cell.pos };
	}
};