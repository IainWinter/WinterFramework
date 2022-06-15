#pragma once

#include "Leveling.h"
#include "Physics.h"
#include "Rendering.h"
#include "Sand/SandEvents.h"
#include "Sand/SandComponents.h"
#include "ext/marching_cubes.h"

struct Sand_System_CreateCollider : System<Sand_System_CreateCollider>
{
	void Init()
	{
		Attach<event_Sand_CreateCollider>();
	}

	void on(event_Sand_CreateCollider& e)
	{
		bool hasCollider = SetPolygonColliderOnSprite(e.entity);

		if (!hasCollider)
		{
			// fail case: if no colliders just explode, not sure why hitbox fails sometimes, 
			//            mostly only with really small shapes, so not a big deal

			Send(event_SandTurnSpriteToDust{ e.entity });
		}
	}

private:

	bool SetPolygonColliderOnSprite(Entity entity)
	{
		auto [tran, body, sand] = entity.GetAll<Transform2D, Rigidbody2D, SandSprite>();

		r<Texture>& mask = sand.colliderMask;

		assert(mask->Channels() == 4 && "collider mask needs to be 32 bit right now");

		auto polygons = MakePolygonFromField<u32>(
			(u32*)mask->Pixels(),
			mask->Width(),
			mask->Height(),
			[](const u32& color) { return (color & 0xff000000) > 0; }
		);

		body.RemoveColliders();

		for (const std::vector<vec2>& polygon : polygons.first)
		{
			b2PolygonShape shape;
			for (int i = 0; i < polygon.size(); i++)
			{
				shape.m_vertices[i] = _tb(polygon.at(i) * tran.scale);
			}
			shape.Set(shape.m_vertices, polygon.size());

			assert(polygon.size() < 12 && "hitbox library genereated a polygon with more than 12 verts, could expand b2 limit or put a limit on the methods in hitbox lib");
			body.AddCollider(shape, sand.density);
		}

		// debug
		// wont work if entity has Mesh
		// need to make a model class with a list of Meshs + Transforms

		std::vector<vec2> debug_mesh;
		for (const std::vector<vec2>& polygon : polygons.first) for (const vec2& v : polygon) debug_mesh.push_back(v);

		if (entity.Has<Mesh>()) entity.Remove<Mesh>();
		entity.Add<Mesh>();
		Mesh& mesh = entity.Get<Mesh>();
		mesh.topology = Mesh::tLoops;
		mesh.Add(Mesh::aPosition, debug_mesh);

		return polygons.first.size() > 0;
	}
};