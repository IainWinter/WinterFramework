#pragma once

#include "app/System.h"
#include "Physics.h"
#include "Rendering.h"
#include "ext/algo/marching_cubes.h"
#include "ext/rendering/Model.h"

#include "CoordTranslation.h"

#include "Sand/SandEvents.h"
#include "Sand/SandComponents.h"
#include "Sand/SandHelpers.h"

struct Sand_System_CreateCollider : System<Sand_System_CreateCollider>
{
	void Init()
	{
		Attach<event_Sand_CreateCollider>();
	}

	void on(event_Sand_CreateCollider& e)
	{
		if (!TrySetCircleColliderOnSprite(e.entity))
		{
			if (!TrySetPolygonColliderOnSprite(e.entity))
			{
				// fail case: if no colliders just explode, not sure why hitbox fails sometimes, 
				log_game("Error: failed to create collider");
				Send(event_Sand_ExplodeToDust{ e.entity });
			}
		}
	}

private:

	bool TrySetCircleColliderOnSprite(Entity entity)
	{
		auto [tran, body, sand] = entity.GetAll<Transform2D, Rigidbody2D, SandSprite>();

		if (sand.isCircle)
		{
			if (body.GetColliderCount() == 0)
			{
				body.AddCollider(b2CircleShape());
			}

			tran.scale = sand.colliderMask->Dimensions() * First<CoordTranslation>().CellsToMeters;
			body.SetTransform(tran);
		}

		return sand.isCircle;
	}

	// need to recalc the cricle

	bool TrySetPolygonColliderOnSprite(Entity entity)
	{
		auto [tran, body, sand] = entity.GetAll<Transform2D, Rigidbody2D, SandSprite>();
		r<Texture>& mask = sand.colliderMask;

		assert(mask->Channels() == 4 && "collider mask needs to be 32 bit right now");
		assert(entity.IsAlive());

		auto polygons = MakePolygonFromField<u32>(
			(u32*)mask->Pixels(),
			mask->Width(),
			mask->Height(),
			[](const u32& color) { return (color & 0xff000000) > 0; }
		);

		auto [minX, minY, maxX, maxY] = GetBoundingBoxOfIsland(sand.pixels.GetAll(), mask->Width()); // just track in CorePixels as added and removed
		
		vec2 sizeIsland = vec2(maxX - minX + 1, maxY - minY + 1);
		vec2 sizeMask   = vec2(mask->Width(), mask->Height());
		
		vec2 offset = sizeMask / sizeIsland - vec2(1.f, 1.f);
		vec2 scale = sizeIsland * First<CoordTranslation>().CellsToMeters;

		// scale and move origin of polygons
		for (std::vector<vec2>& polygon : polygons.first)
		for (vec2&              vert    : polygon)
		{
			vert = (vert - offset) * scale;
		}
		
		body.RemoveColliders();
		for (const std::vector<vec2>& polygon : polygons.first)
		{
			b2PolygonShape shape;
			for (int i = 0; i < polygon.size(); i++)
			{
				shape.m_vertices[i] = _tb(polygon.at(i));
			}
			shape.Set(shape.m_vertices, (int)polygon.size());

  			assert(polygon.size() <= 12 && "hitbox library genereated a polygon with more than 12 verts, could expand b2 limit or put a limit on the methods in hitbox lib");
			body.AddCollider(shape, sand.density);
		}

		if (entity.Has<Model>()) entity.Remove<Model>();
		
		Model& model = entity.Add<Model>();

		for (const std::vector<vec2>& polygon : polygons.first)
		{
			std::vector<vec2> debug_mesh;
			std::vector<vec4> debug_mesh_colors;

			for (const vec2& v : polygon) 
			{
				debug_mesh.push_back(v / tran.scale); 
				debug_mesh_colors.push_back(vec4(1.f, 1.f, 1.f, 1.f));
			}

			r<Mesh> mesh = mkr<Mesh>();
			mesh->topology = Mesh::tLoops;
			mesh->Add(Mesh::aPosition, debug_mesh);
			mesh->Add(Mesh::aColor,    debug_mesh_colors);

			model.meshes.push_back(mesh);
		}

		return polygons.first.size() > 0;
	}
};