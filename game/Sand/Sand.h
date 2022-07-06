#pragma once

#include "Rendering.h"
#include "Entity.h"
#include "Physics.h"
#include "Leveling.h"
#include "Windowing.h"
#include "Events.h"

#include "ext/Time.h"
#include "ext/flood_fill.h"
#include "ext/marching_cubes.h"
#include "ext/Components.h"
#include "ext/rendering/Sprite.h"
#include "ext/rendering/Camera.h"

#include "Components/KeepOnScreen.h"
#include "CoordTranslation.h"

#include "Sand/SandEvents.h"
#include "Sand/SandComponents.h"
#include "Sand/SandHelpers.h"

#include <vector>
#include <functional>
#include <unordered_set>

struct CellCollisionInfo
{
	ivec2 spriteHitIndex;
	int spriteEntityID;
	int hasHit;
};

struct SandWorld
{
	ivec2 worldSizeCells;
	vec2 cameraSizeMeters;
	float cellsPerMeter;

	r<Target> screenRead;

	SandWorld(int cellsPerMeter, vec2 camSize)
	{
		ResizeWorld(cellsPerMeter, camSize);
		screenRead = mkr<Target>(false);
		screenRead->Add(Target::aColor,  worldSizeCells.x, worldSizeCells.y, Texture::uINT_32, false);
	}

	void ResizeWorld(int cellsPerMeter_, vec2 camSize)
	{
		worldSizeCells = camSize * 2.f * (float)cellsPerMeter_;
		cameraSizeMeters = camSize;
		cellsPerMeter = cellsPerMeter_;
	}

	ivec2 ToScreenPos(vec2 posInMeters) const
	{
		return posInMeters * cellsPerMeter + vec2(worldSizeCells) / 2.f;
	}

	bool CollidePixel(vec2 pos) /*const*/
	{
		return GetCollisionInfo(pos).hasHit;
	}

	CellCollisionInfo& GetCollisionInfo(vec2 pos) /*const*/
	{
		ivec2 p = ToScreenPos(pos);
		return *(CellCollisionInfo*)screenRead->Get(Target::aColor)->At<int>(p.x, p.y);
	}

	bool OnScreen(vec2 pos) const
	{
		ivec2 p = ToScreenPos(pos);

		return p.x >= 0 
			&& p.y >= 0
			&& p.x < worldSizeCells.x 
			&& p.y < worldSizeCells.y;
	}

	// yes moves
	//  no copys
	SandWorld(SandWorld&& move) = default;
	SandWorld& operator=(SandWorld&& move) = default;
	SandWorld(const SandWorld& copy) = delete;
	SandWorld& operator=(const SandWorld& copy) = delete;
};

// rendering for collision info

// sand update

// render all the tiles to a texture, could chunk but regolith is a single screen game so dont worry about this for now
// render bullets

struct Sand_System_Update : System<Sand_System_Update>
{	
	void Init()
	{
		//Attach<event_WindowResize>();
		Attach<event_SandAddSprite>();
		//Attach<event_Sand_CreateCell>();

		//auto [sand, camera] = GetModules<SandWorld, Camera>();
		
		//CreateEntity().AddAll(
		//	Transform2D(vec2(0.f), vec2(5, 5)), 
		//	Sprite(sand.screenRead->Get(Target::aColor))
		//);
	}

	//void on(event_Sand_CreateCell& e)
	//{
	//	Entity entity = CreateEntity();
	//	entity.Add<Transform2D>(e.cell.pos);
	//	GetModule<PhysicsWorld>().AddEntity(entity).SetVelocity(e.cell.vel);

	//	if (e.onCreate)
	//	{
	//		e.onCreate(entity);
	//	}
	//}

	//void on(event_WindowResize& e)
	//{
	//	auto [sand, camera] = GetModules<SandWorld, Camera>();
	//	sand.ResizeWorld(sand.cellsPerMeter, vec2(camera.w, camera.h));
	//}

	void on(event_SandAddSprite& e)
	{
		Texture& sprite = e.entity.Get<Sprite>().Get();
		Transform2D& transform = e.entity.Get<Transform2D>();
		transform.scale = sprite.Dimensions() / GetModule<SandWorld>().cellsPerMeter;

		SandSprite& sandSprite = e.entity.Get<SandSprite>();

		auto [coreIndex, filled, isHealth, hasAny] = GetCorePixels(e.entity.Get<Sprite>().source);

		sandSprite.core = coreIndex;
		sandSprite.density = e.density;
		sandSprite.cellCount = filled.size();

		// setup collider if requested, odd logic

		Rigidbody2D* body;

		if (e.entity.Has<Rigidbody2D>() && !e.entity.Get<Rigidbody2D>().InWorld())
		{
			body = &GetModule<PhysicsWorld>().AddEntity(e.entity);
		}

		else
		{
			body = &GetModule<PhysicsWorld>().AddEntity(e.entity);
			body->SetVelocity(e.velocity);
			body->SetAngularVelocity(e.aVelocity);
		}

		if (body->GetColliderCount() == 0)
		{
			SendNow(event_Sand_CreateCollider{ e.entity });
		}
	}
};