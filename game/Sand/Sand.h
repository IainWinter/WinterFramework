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
#include "ext/rendering/Particle.h"

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

struct Sand_System_Update : System<Sand_System_Update>
{	
	void Init()
	{
		Attach<event_SandAddSprite>();
	}

	void Update()
	{
		SandWorld& sand = GetModule<SandWorld>();

		// turn off emitter when off screen

		for (auto [transform, particle, cell] : Query<Transform2D, ParticleEmitter, CellProjectile>())
		{
			if (!sand.OnScreen(transform.position))
			{
				cell.timeOffscreen += Time::DeltaTime();
				particle.enableAutoEmit = cell.timeOffscreen < .1f; // wait a few frames to allow some particles from offscreen, and stop early delete
			}
		}

		// remove sand sprites that are off screen

		for (auto [entity, transform, sprite] : QueryWithEntity<Transform2D, SandSprite>())
		{
			if (sand.OnScreen(transform.position))
			{
				sprite.hasBeenOnScreenOnce = true;
			}

			else if (sprite.hasBeenOnScreenOnce)
			{
				sprite.timeOffscreen += Time::DeltaTime();
				if (sprite.timeOffscreen > 4.f)
				{
					entity.DestroyAtEndOfFrame();
				}
			}
		}
	}

	void on(event_SandAddSprite& e)
	{
		r<Texture> sprite = e.entity.Get<Sprite>().source;
		e.entity.Get<Transform2D>().scale = sprite->Dimensions() / GetModule<SandWorld>().cellsPerMeter;
		
		SandSprite& sandSprite = e.entity.Get<SandSprite>();

		sandSprite.pixels = GetCorePixels(sprite);
		sandSprite.initalCore = sandSprite.pixels.GetCoreAsVec();
		sandSprite.density = e.density;

		Rigidbody2D* body;
	
		if (e.entity.Has<Rigidbody2D>())
		{
			body = &e.entity.Get<Rigidbody2D>();
		}

		else
		{
			body = &GetModule<PhysicsWorld>().AddEntity(e.entity);
			body->SetVelocity(e.velocity);
			body->SetAngularVelocity(e.aVelocity);
		}

		e.entity.Add<WrapOnScreen>();

		SendNow(event_Sand_CreateCollider{ e.entity });
	}
};