#pragma once

#include "Rendering.h"
#include "Entity.h"
#include "ext/Time.h"
#include <vector>
#include <functional>

enum class CellMovement
{
	MOVE_FORWARD
};

struct Cell
{
	vec2 pos;
	vec2 vel;
	float dampen = 20;
	Color color;
};

struct CellLife
{
	float life;
};

struct SandSprite
{
};

struct event_SandCellCollision
{
	entity projectile;
	entity sprite;
	ivec2 hitPosInSprite;
};

struct SandWorld
{
	entity display;

	vec2 worldScale; // scale of meters to cells
	ivec2 screenSize;
	vec2 screenOffset;

	r<Target> screen;
	std::vector<entity> tileCache;

	SandWorld() = default;

	SandWorld(int w, int h, int camScaleX, int camScaleY)
	{
		screen = std::make_shared<Target>();

		//spriteTarget->Add(Target::aDepth, w, h, 1);
		screen->Add(Target::aColor, w, h, 4, false);
		screen->Add(Target::aColor1, w, h, 4, false);

		display = entities().create()
			.add<Transform2D>(0, 0, 0, camScaleX, camScaleY, 0)
			.add<Sprite>(screen->Get(Target::aColor))
			.add<Renderable>();

		worldScale.x = w / camScaleX / 2; // by 2 bc mesh does from -1 to 1
		worldScale.y = h / camScaleY / 2;

		screenSize.x = w;
		screenSize.y = h;

		screenOffset = screenSize / 2;
	}

	~SandWorld()
	{
		display.destroy();
	}

	entity CreateCell(float x, float y, Color color, float vx = 0, float vy = 0, float dampen = 20.f)
	{
		return entities().create().add<Cell>(vec2(x * worldScale.x, y * worldScale.y), vec2(vx, vy), dampen, color);
	}

	void Update()
	{
		Texture& color = *screen->Get(Target::aColor);
		Texture& sInfo = *screen->Get(Target::aColor1); // sprite info / collision info, probally an index (or entity index) to an array of structs

		color.SendToHost();
		sInfo.SendToHost();

		for (auto [e, cell] : entities().query<Cell>().with_entity())
		{
			DrawLine(color, sInfo, e, cell);
		}

		color.SendToDevice();
	}

private:
	void DrawLine(Texture& display, Texture& collisionInfo, entity e, Cell& cell)
	{
		vec2 delta = cell.vel / cell.dampen;
		vec2 current = cell.pos;

		float distance = glm::length(delta);
		delta /= distance;

		for (int i = 0; i < ceil(distance); i++)
		{
			ivec2 raster = floor(current + screenOffset);

			if (!OnScreen(raster))
			{
				// break if going offscreen
				continue;
			}

			if (CollidePixel(collisionInfo, raster))
			{
				Color spriteInfo = collisionInfo.At(raster.x, raster.y);
				ivec2 positionInSprite = ivec2(spriteInfo.r, spriteInfo.g);
				int tileIndex = spriteInfo.b;

				events().send(event_SandCellCollision { e, tileCache.at(tileIndex), positionInSprite });
				break;
			}

			DrawPixel(display, floor(current + screenOffset), cell.color);
			current += delta;
		}
	}

	void DrawPixel(Texture& display, ivec2 pos, Color c)
	{
		display.At(pos.x, pos.y) = c;
	}

	bool CollidePixel(Texture& collisionInfo, ivec2 pos)
	{
		if (collisionInfo.At(pos.x, pos.y).a > 0)
		{
			return true;
		}

		return false;
	}

	bool OnScreen(ivec2 pos)
	{
		return pos.x > 0 && pos.y > 0 && pos.x < screenSize.x && pos.y < screenSize.y;
	}
};

// sand update

// render all the tiles to a texture, could chunk but regolith is a single screen game so dont worry about this for now
// render bullets

struct Sand_System_RenderTiles : System
{
	void Update()
	{
		auto [render, camera, sand] = Get<SpriteRenderer2D, Camera, SandWorld>();

		sand.tileCache.clear(); // bad for a system to touch state like this... kinda a hack only because entity handle cant turn into a u32

		render.Begin(camera, sand.screen);
		render.Clear(Color(0));

		int spriteIndex = 0;
		for (auto [e, transform, sprite] : entities().query<Transform2D, Sprite, SandSprite>().only<Transform2D, Sprite>().with_entity())
		{
			render.m_shader.Set("spriteIndex", spriteIndex);
			sand.tileCache.push_back(e);
			spriteIndex += 1;

			render.DrawSprite(transform, sprite.Get());
		}

		sand.Update();
	}
};

struct Sand_LifeUpdateSystem : System
{
	void Update()
	{
		for (auto [e, time] : entities().query<CellLife>().with_entity())
		{
			time.life -= Time::DeltaTime();
			if (time.life <= 0.f)
			{
				entities_defer().destroy(e);
			}
		}
	}
};

struct Sand_VelUpdateSystem : System
{
	void Update() override
	{
		for (auto [cell] : entities().query<Cell>())
		{
			cell.pos += cell.vel * Time::DeltaTime();
		}
	}
};

struct Sand_System_CollisionVel : System
{
	 Sand_System_CollisionVel() { events().attach<event_SandCellCollision>(this); }
	~Sand_System_CollisionVel() { events().detach(this); }

	void on(event_SandCellCollision& e)
	{
		vec2& vel = e.projectile.get<Cell>().vel;
		float speed = length(vel);
		vel.x += get_rand(400) - 200;
		vel.y += get_rand(400) - 200;
		vel = normalize(vel) * speed;
	}
};
