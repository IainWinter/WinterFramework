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
#include <vector>
#include <functional>
#include <unordered_set>

#include "Components/KeepOnScreen.h"

#include "Sand/SandEvents.h"
#include "Sand/SandComponents.h"
#include "Sand/SandHelpers.h"

struct SandWorld
{
	Entity display;

	 vec2 worldScaleInit; // scale of meters to cells, needed for sprite cutting
	 vec2 worldScale;     // scale of meters to cells
	 vec2 screenOffset;
	ivec2 screenSize;

	r<Target> screenRead;
	r<Target> screenWrite; // to make copying to cpu not stall?

	SandWorld(int w, int h, int camScaleX, int camScaleY)
	{
		screenRead = mkr<Target>(false);
		screenRead->Add(Target::aColor, w, h, Texture::uINT_32, false);

		screenWrite = mkr<Target>(false);
		screenWrite->Add(Target::aColor, w, h, Texture::uINT_32, false);

		Mesh LPmesh = Mesh(false);
		LPmesh.Add<vec2>(Mesh::aPosition, {});
		LPmesh.Add<vec4>(Mesh::aColor, {});
		LPmesh.topology = Mesh::tLines;

		ResizeWorld(w, h, camScaleX, camScaleY);
		worldScaleInit = worldScale;
	}

	void ResizeWorld(int width, int height, int camScaleX, int camScaleY)
	{
		screenRead ->Resize(width, height);
		screenWrite->Resize(width, height);
		
		screenSize = vec2(width, height);
		screenOffset = screenSize / 2;

		worldScale.x = width  / camScaleX / 2; // by 2 bc mesh does from -1 to 1
		worldScale.y = height / camScaleY / 2;
	}

	void DrawPixel(Texture& display, ivec2 pos, Color c)
	{
		pos += screenOffset;
		display.At(pos.x, pos.y) = c;
	}

	bool CollidePixel(ivec2 pos) const
	{
		return GetCollisionInfo(pos)[3] > 0;
	}

	const ivec4& GetCollisionInfo(ivec2 pos) const
	{
		pos += screenOffset;
		return *(ivec4*)screenRead->Get(Target::aColor)->At<int>(pos.x, pos.y);
	}

	vec2 ToScreenPos(const vec2& pos) const
	{
		return pos * worldScale;
	}

	bool OnScreen(ivec2 pos) const
	{
		pos += screenOffset; 
		return pos.x >= 0 && pos.y >= 0 && pos.x < screenSize.x && pos.y < screenSize.y;
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
		Attach<event_WindowResize>();
		Attach<event_SandAddSprite>();
		Attach<event_Sand_CreateCell>();

		//maskRender = mkr<SandCollisionInfoRenderer>();
	}

	void on(event_Sand_CreateCell& e)
	{
		Cell c = e.cell;
		if (e.adjustPosition)
		{
			c.pos = GetModule<SandWorld>().ToScreenPos(c.pos);
		}

		Entity entity = CreateEntity().AddAll(c);
		
		if (e.onCreate)
		{
			e.onCreate(entity);
		}
	}

	void on(event_WindowResize& e)
	{
		auto [sand, camera] = GetModules<SandWorld, Camera>();
		sand.ResizeWorld(e.width, e.height, camera.w, camera.h);
	}

	void on(event_SandAddSprite& e)
	{
		if (!e.entity.Has<KeepOnScreen>())
			e.entity.Add<KeepOnScreen>();

		Texture& sprite = e.entity.Get<Sprite>().Get();
		Transform2D& transform = e.entity.Get<Transform2D>();
		transform.scale.x = sprite.Width()  / GetModule<SandWorld>().worldScaleInit.x;
		transform.scale.y = sprite.Height() / GetModule<SandWorld>().worldScaleInit.y;

		SandSprite& sandSprite = e.entity.Get<SandSprite>();

		auto [coreIndex, filled, isHealth, hasAny] = GetCorePixels(e.entity.Get<Sprite>().source);

		sandSprite.core = coreIndex;
		sandSprite.density = e.density;
		sandSprite.cellCount = filled.size();

		// setup collider if requested

		Rigidbody2D& body = GetModule<PhysicsWorld>().AddEntity(e.entity);
		body.SetVelocity(e.velocity);
		body.SetAngularVelocity(e.aVelocity);

		if (body.GetColliderCount() == 0)
		{
			SendNow(event_Sand_CreateCollider{ e.entity });
		}
	}
};